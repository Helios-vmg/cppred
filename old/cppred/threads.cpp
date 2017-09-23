#include "threads.h"

void Event::signal(){
	std::lock_guard<std::mutex> lock(this->mutex);
	this->signalled = true;
	this->cv.notify_one();
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
	{
		std::unique_lock<std::mutex> lock(this->mutex);
		this->signalled = false;
	}
	return this->wait_for(ms);
}

bool Event::wait_for(unsigned ms){
	std::cv_status result;
	{
		std::unique_lock<std::mutex> lock(this->mutex);
		if (this->signalled){
			this->signalled = false;
			return true;
		}
		result = this->cv.wait_for(lock, std::chrono::milliseconds(ms));
	}
	return result == std::cv_status::no_timeout;
}
