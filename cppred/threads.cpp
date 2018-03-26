#include "stdafx.h"
#include "threads.h"
#include "utility.h"

void Event::signal(){
	LOCK_MUTEX(this->mutex);
	this->signalled = true;
	this->cv.notify_one();
}

bool Event::state(){
	LOCK_MUTEX(this->mutex);
	return this->signalled;
}

void Event::reset(){
	std::unique_lock<std::mutex> lock(this->mutex);
	this->signalled = false;
}

bool Event::wait_impl(){
	std::unique_lock<std::mutex> lock(this->mutex);
	if (this->signalled){
		this->signalled = false;
		return false;
	}
	this->cv.wait(lock);
	return true;
}

void Event::wait(){
	while (this->wait_impl());
}

void Event::reset_and_wait(){
	this->wait_impl();
	this->wait();
}

bool Event::reset_and_wait_for(unsigned ms){
	std::cv_status result;
	HighResolutionClock clock;
	auto target = clock.get() + ms * 0.001;
	bool first = true;
	do{
		std::unique_lock<std::mutex> lock(this->mutex);
		if (first){
			if (this->signalled)
				this->signalled = false;
			first = false;
		}else{
			if (this->signalled){
				this->signalled = false;
				return true;
			}
		}
		result = this->cv.wait_for(lock, std::chrono::milliseconds((unsigned)((target - clock.get()) * 1000)));
		if (result == std::cv_status::timeout)
			return false;
	}while (clock.get() < target);
	return false;
}

bool Event::wait_for(unsigned ms){
	std::cv_status result;
	HighResolutionClock clock;
	auto target = clock.get() + ms * 0.001;
	do{
		std::unique_lock<std::mutex> lock(this->mutex);
		if (this->signalled){
			this->signalled = false;
			return true;
		}
		result = this->cv.wait_for(lock, std::chrono::milliseconds((unsigned)((target - clock.get()) * 1000)));
		if (result == std::cv_status::timeout)
			return false;
	}while (clock.get() < target);
	return false;
}
