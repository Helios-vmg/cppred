#include "HighResolutionClock.h"
#include "utility.h"
#include <iostream>

#if (defined _WIN32 || defined _WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::uint64_t get_timer_resolution(){
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
}

std::uint64_t get_timer_count(){
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return count.QuadPart;
}
#else
#include <chrono>

std::uint64_t get_timer_resolution(){
	typedef std::chrono::high_resolution_clock::period T;
	return T::den / T::num;
}

std::uint64_t get_timer_count(){
	auto now = std::chrono::high_resolution_clock::now();
	return now.time_since_epoch().count();
}
#endif

HighResolutionClock::HighResolutionClock(){
	this->reference_time = get_timer_count();
	this->resolution = 1.0 / get_timer_resolution();
}

double HighResolutionClock::get(){
	return (get_timer_count() - this->reference_time) * this->resolution;
}

void SteppingClock::step(){
	if (this->reference_time < 0){
		this->reference_time = this->parent_clock->get();
		this->current_time = 0;
	}else
		this->current_time = this->parent_clock->get() - this->reference_time;
}

void FixedClock::step(){
	this->current_time = this->clock++ * (1.0 / 60.0);
}

void PausableClock::step(){
	if (this->reference_time < 0){
		this->reference_time = this->parent_clock->get();
		this->current_time = this->accumulated_time;
		this->paused = false;
	}else{
		if (this->paused){
			this->resume();
			return;
			//throw std::runtime_error("PausableClock used incorrectly. step() called while in the paused state.");
		}
		this->current_time = this->accumulated_time + this->parent_clock->get() - this->reference_time;
	}
}

void PausableClock::pause(){
	if (this->paused)
		return;
	assert(this->reference_time >= 0);
	this->accumulated_time += this->parent_clock->get() - this->reference_time;
	this->paused = true;
}

void PausableClock::resume(){
	if (!this->paused)
		return;
	//Logger() << "PausableClock::resume()\n";
	this->reference_time = -1;
	this->step();
}
