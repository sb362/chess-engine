#include "thread.hh"

using namespace threading;

Thread::Thread(std::size_t thread_id)
	: thread_id(thread_id), cv(), mutex(), stop(false), idle(false), quit(false),
	  thread(&Thread::loop, this)
{
	wait_until_idle();
}

Thread::~Thread()
{
	stop_thinking();
	wait_until_idle();

	mutex.lock();
	quit = true;
	idle = false;

	cv.notify_one();
	mutex.unlock();

	thread.join();
}

std::size_t Thread::id() const
{
	return thread_id;
}

bool Thread::is_idle()
{
	std::lock_guard lock {mutex};
	return idle;
}

bool Thread::should_stop()
{
	return stop;
}

void Thread::wait_until_idle()
{
	std::unique_lock lock {mutex};
	cv.wait(lock, [&] { return idle; });
}

void Thread::start_thinking()
{
	std::lock_guard lock {mutex};

	stop = false;
	idle = false;

	cv.notify_one();
}

void Thread::stop_thinking()
{
	stop = true;
}

void Thread::loop()
{
	std::unique_lock lock {mutex, std::defer_lock};

	while (true)
	{
		lock.lock();

		idle = true;
		cv.notify_one();
		cv.wait(lock, [&] { return !idle; });

		if (quit)
			break;

		lock.unlock();

		think();
	}
}
