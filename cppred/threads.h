#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

typedef std::lock_guard<std::mutex> automutex_t;

class Event{
	bool signalled = false;
	std::mutex mutex;
	std::condition_variable cv;
	bool wait_impl();
public:
	void signal();
	void reset_and_wait();
	bool reset_and_wait_for(unsigned ms);
	void wait();
	bool wait_for(unsigned ms);
	void reset();
};

inline bool join_thread(std::unique_ptr<std::thread> &t){
	if (!t)
		return false;
	t->join();
	t.reset();
	return true;
}
