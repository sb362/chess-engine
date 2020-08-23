#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace threading
{
	inline unsigned max_threads()
	{
		return std::thread::hardware_concurrency();
	}

	struct SpinLock
	{
		std::atomic_flag flag;

		void lock()
		{
			while (flag.test_and_set(std::memory_order_acquire)) {}
		}

		void unlock()
		{
			flag.clear(std::memory_order_release);
		}
	};

	class Thread
	{
	private:
		const std::size_t thread_id;
		std::condition_variable cv;
		std::mutex mutex; // Mutex for cv, idle, quit

		std::atomic<bool> stop;
		bool idle, quit;

		std::thread thread;

	public:
		Thread(std::size_t thread_id = 0);
		virtual ~Thread();

		std::size_t id() const;

		bool is_idle();
		bool should_stop();

		void wait_until_idle();

		void start_thinking();
		void stop_thinking();

		virtual void think() = 0;

	private:
		void loop();
	};
} // threading
