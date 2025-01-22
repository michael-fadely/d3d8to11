#include "pch.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t thread_count)
	: m_thread_count(thread_count),
	  m_running(false),
	  m_pending_task_count(0)
{
	start();
}

ThreadPool::~ThreadPool()
{
	shutdown();
}

size_t ThreadPool::thread_count() const
{
	return m_thread_count;
}

void ThreadPool::start()
{
	if (m_running == true)
	{
		return;
	}

	m_running = true;
	m_threads.reserve(m_thread_count);

	for (size_t i = 0; i < m_thread_count; ++i)
	{
		m_threads.emplace_back(&ThreadPool::thread_function, this);
	}
}

void ThreadPool::shutdown()
{
	if (m_running == false)
	{
		return;
	}

	m_running = false;
	m_pending_task_cv.notify_all();

	for (std::thread& t : m_threads)
	{
		t.join();
	}

	m_threads.clear();
}

void ThreadPool::wait()
{
	std::unique_lock queue_lock(m_queue_mutex);
	auto wait_predicate = [this] { return m_running == false || m_queue.empty(); };
	m_tasks_complete_cv.wait(queue_lock, wait_predicate);
}

void ThreadPool::enqueue_internal(std::unique_ptr<FunctionWrapperBase> wrapper)
{
	{
		std::lock_guard queue_lock(m_queue_mutex);
		m_queue.emplace(std::move(wrapper));
		++m_pending_task_count;
	}

	m_pending_task_cv.notify_one();
}

void ThreadPool::thread_function()
{
	auto wait_predicate = [this] { return !m_queue.empty() || m_running == false; };

	while (true)
	{
		std::unique_lock queue_lock(m_queue_mutex);
		m_pending_task_cv.wait(queue_lock, wait_predicate);

		// don't actually exit the loop until all queue items have been consumed
		if (m_queue.empty() && m_running == false)
		{
			break;
		}

		auto task = std::move(m_queue.front());
		m_queue.pop();
		queue_lock.unlock();

		(*task)();

		if (--m_pending_task_count == 0)
		{
			m_tasks_complete_cv.notify_all();
		}
	}
}
