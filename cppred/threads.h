#pragma once

#ifndef HAVE_PCH
#include <condition_variable>
#include <mutex>
#include <thread>
#endif

class Event{
	bool signalled = false;
	std::mutex mutex;
	std::condition_variable cv;
	bool wait_impl();
public:
	void signal();
	void reset_and_wait();
	//Returns false if the wait period expired.
	bool reset_and_wait_for(unsigned ms);
	void wait();
	//Returns false if the wait period expired.
	bool wait_for(unsigned ms);
	void reset();
	bool state();
};

inline bool join_thread(std::unique_ptr<std::thread> &t){
	if (!t)
		return false;
	t->join();
	t.reset();
	return true;
}
