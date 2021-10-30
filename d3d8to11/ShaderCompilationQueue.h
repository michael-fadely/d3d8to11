#pragma once

#include <queue>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "hash_combine.h"

enum class ShaderCompilationType
{
	none,
	vertex,
	pixel
};

struct ShaderCompilationQueueEntry
{
	ShaderCompilationType type;
	ShaderFlags::type flags;

	ShaderCompilationQueueEntry();
	ShaderCompilationQueueEntry(ShaderCompilationType type, ShaderFlags::type flags);

	bool operator==(const ShaderCompilationQueueEntry& rhs) const;
	bool operator!=(const ShaderCompilationQueueEntry& rhs) const;
};

namespace std
{
	template <>
	struct hash<ShaderCompilationQueueEntry>
	{
		std::size_t operator()(const ShaderCompilationQueueEntry& s) const noexcept
		{
			size_t h = std::hash<size_t>()(static_cast<size_t>(s.type));
			hash_combine(h, s.flags);
			return h;
		}
	};
}

class ShaderCompilationQueue
{
	using CompilationFunction = std::function<void(const ShaderCompilationQueueEntry& entry)>;

	const size_t _thread_count;
	std::atomic_bool _running;
	std::condition_variable _condition;
	std::mutex _mutex;
	std::vector<std::thread> _threads;
	std::unordered_map<ShaderCompilationQueueEntry, CompilationFunction> _functions;
	std::queue<ShaderCompilationQueueEntry> _queue;

	size_t _active_threads = 0;
	size_t _enqueued_count = 0;

public:
	explicit ShaderCompilationQueue(size_t thread_count);
	~ShaderCompilationQueue();

	void enqueue(ShaderCompilationType type, ShaderFlags::type flags, CompilationFunction function);
	void start();
	void shutdown();

	[[nodiscard]] size_t active_threads() const;
	[[nodiscard]] size_t enqueued_count() const;

private:
	void work_thread();
};
