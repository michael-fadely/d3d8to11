#include "stdafx.h"
#include "ShaderCompilationQueue.h"

ShaderCompilationQueueEntry::ShaderCompilationQueueEntry()
	: type(ShaderCompilationType::none),
	  flags(ShaderFlags::none)
{
}

ShaderCompilationQueueEntry::ShaderCompilationQueueEntry(ShaderCompilationType type, ShaderFlags::type flags)
	: type(type),
	  flags(flags)
{
}

bool ShaderCompilationQueueEntry::operator==(const ShaderCompilationQueueEntry& rhs) const
{
	return type == rhs.type && flags == rhs.flags;
}

bool ShaderCompilationQueueEntry::operator!=(const ShaderCompilationQueueEntry& rhs) const
{
	return type != rhs.type || flags != rhs.flags;
}

ShaderCompilationQueue::ShaderCompilationQueue(size_t thread_count)
	: _thread_count(thread_count),
	  _running(true),
	  _threads(_thread_count),
	  _functions(),
	  _queue()
{
	for (std::thread& t : _threads)
	{
		t = std::thread(&ShaderCompilationQueue::work_thread, this);
	}
}

ShaderCompilationQueue::~ShaderCompilationQueue()
{
	shutdown();
}

void ShaderCompilationQueue::enqueue(ShaderCompilationType type, ShaderFlags::type flags, CompilationFunction function)
{
	std::lock_guard guard(_mutex);

	auto key = ShaderCompilationQueueEntry(type, flags);
	const auto it = _functions.find(key);

	if (it != _functions.end())
	{
		return;
	}

	++_enqueued_count;
	_queue.emplace(key);
	_functions[key] = std::move(function);
	_condition.notify_one();

	std::stringstream ss;
	ss << "active threads: " << _active_threads << "; enqueued shaders: " << _enqueued_count << "\n";
	OutputDebugStringA(ss.str().c_str());
}

void ShaderCompilationQueue::start()
{
	if (_running == true)
	{
		return;
	}

	_running = true;

	for (std::thread& t : _threads)
	{
		t = std::thread(&ShaderCompilationQueue::work_thread, this);
	}
}

void ShaderCompilationQueue::shutdown()
{
	if (_running == true)
	{
		return;
	}

	_running = false;
	_condition.notify_all();

	for (std::thread& t : _threads)
	{
		t.join();
	}
}

size_t ShaderCompilationQueue::active_threads() const
{
	return _active_threads;
}

size_t ShaderCompilationQueue::enqueued_count() const
{
	return _enqueued_count;
}

void ShaderCompilationQueue::work_thread()
{
	while (_running == true)
	{
		ShaderCompilationQueueEntry key;
		CompilationFunction function;

		{
			if (_running == false)
			{
				break;
			}

			std::unique_lock lock(_mutex);

			if (_queue.empty())
			{
				_condition.wait(lock);

				if (_queue.empty())
				{
					continue;
				}
			}

			++_active_threads;
			std::stringstream ss;
			ss << "active threads: " << _active_threads << "; enqueued shaders: " << _enqueued_count << "\n";
			OutputDebugStringA(ss.str().c_str());

			key = _queue.front();
			_queue.pop();

			function = std::move(_functions[key]);
		}

		function(key);

		{
			std::unique_lock lock(_mutex);
			_functions.erase(key);

			--_enqueued_count;
			--_active_threads;
			std::stringstream ss;
			ss << "active threads: " << _active_threads << "; enqueued shaders: " << _enqueued_count << "\n";
			OutputDebugStringA(ss.str().c_str());
		}
	}
}
