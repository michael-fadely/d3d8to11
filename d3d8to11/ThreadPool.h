#pragma once

#include <atomic>
#include <chrono> // used by is_future_ready
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

class ThreadPool
{
public:
	explicit ThreadPool(size_t thread_count);
	ThreadPool(ThreadPool&& other) noexcept = delete;
	ThreadPool(const ThreadPool& other) = delete;
	~ThreadPool();

	ThreadPool& operator=(ThreadPool&& rhs) noexcept = delete;
	ThreadPool& operator=(const ThreadPool& rhs) = delete;

	[[nodiscard]] size_t thread_count() const;

	void start();
	void shutdown();

	void wait();

	template <typename Func, typename... Args>
	[[nodiscard]] auto enqueue(Func func, Args&&... args);

private:
	class FunctionWrapperBase
	{
	public:
		virtual ~FunctionWrapperBase() = default;
		virtual void operator()() = 0;
	};

	template <typename Func>
	class FunctionWrapper : public FunctionWrapperBase
	{
	public:
		explicit FunctionWrapper(Func&& function)
			: m_function(std::forward<Func>(function))
		{
		}

		~FunctionWrapper() override = default;

		FunctionWrapper(const FunctionWrapper&) = delete;
		FunctionWrapper(FunctionWrapper&&) noexcept = default;

		FunctionWrapper& operator=(const FunctionWrapper&) = delete;
		FunctionWrapper& operator=(FunctionWrapper&&) noexcept = default;

		void operator()() override
		{
			m_function();
		}

	private:
		Func m_function;
	};

	void enqueue_internal(std::unique_ptr<FunctionWrapperBase> wrapper);
	void thread_function();

	size_t m_thread_count;
	std::vector<std::thread> m_threads;

	std::atomic_bool m_running;
	std::mutex m_queue_mutex;
	std::condition_variable m_pending_task_cv;
	std::atomic_size_t m_pending_task_count;
	std::queue<std::unique_ptr<FunctionWrapperBase>> m_queue;
	std::condition_variable m_tasks_complete_cv;
};

template <typename Func, typename... Args>
auto ThreadPool::enqueue(Func func, Args&&... args)
{
	using invoke_result_t = std::invoke_result_t<Func, Args...>;

	std::promise<invoke_result_t> promise;
	auto future = promise.get_future();

	auto wrapper_fn =
		[
			func     = std::move(func),
			...largs = std::forward<Args>(args),
			promise  = std::move(promise)
		]() mutable
		{
			if constexpr (std::is_void_v<invoke_result_t>)
			{
				func(std::forward<Args>(largs)...);
				promise.set_value();
			}
			else
			{
				promise.set_value(func(std::forward<Args>(largs)...));
			}
		};

	std::unique_ptr<FunctionWrapperBase> wrapper(new FunctionWrapper(std::move(wrapper_fn)));
	enqueue_internal(std::move(wrapper));

	return future;
}

// TODO: move is_future_ready somewhere else
template <typename T>
static bool is_future_ready(std::future<T>& future)
{
	return future.wait_for(std::chrono::milliseconds::zero()) == std::future_status::ready;
}
