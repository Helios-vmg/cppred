#include "SystemClock.h"

bool SystemClock::lock_clock_value(double seconds){
	if (this->t0 < 0){
		this->t0 = seconds;
		this->realtime_clock = 0;
		this->cpu_clock = 0;
		this->DIV_register = 0;
		return true;
	}
	return this->advance_clock((std::uint64_t)((seconds - this->t0) * gb_cpu_frequency));
}

bool SystemClock::advance_clock(std::uint64_t now){
	auto delta = now - this->realtime_clock;
	delta -= delta % 4;
	if (!delta)
		return false;
	this->realtime_clock += delta;
	this->cpu_clock += delta;
	this->DIV_register += delta;
	return true;
}
