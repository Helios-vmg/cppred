#pragma once

#include "CommonTypes.h"

class CppRed;

const unsigned gb_cpu_frequency_power = 22;
const unsigned gb_cpu_frequency = 1 << gb_cpu_frequency_power; //4194304
const double gb_cpu_clock_period_us = 1.0 / ((double)gb_cpu_frequency * 1e-6);
const int dmg_dma_transfer_length_clocks = 640;

class SystemClock{
	CppRed *system;
	//Initialization time.
	double t0 = -1;
	//Ticks at a rate of 2^22 cycles per emulation second (2^23 cycles per
	//emulation second when in double speed mode), regardless of the state of
	//the CPU, as long as the emulation is running.
	//Speeding up or slowing down the emulation increases or decreases the
	//frequency by the same amount.
	std::uint64_t realtime_clock = 0;
	//Ticks at the same rate as realtime_clock, unless the CPU is in the
	//stopped state (caused by a STOP instruction).
	std::uint64_t cpu_clock = 0;
	std::uint32_t DIV_register = 0;

	bool advance_clock(std::uint64_t now);
public:
	SystemClock(CppRed &system): system(&system){}

	bool lock_clock_value(double seconds);
	std::uint64_t get_realtime_clock_value() const{
		return this->realtime_clock;
	}
	std::uint64_t get_clock_value() const{
		return this->cpu_clock;
	}
	byte_t get_DIV_register() const{
		auto ret = this->DIV_register;
		ret >>= 8;
		ret &= 0xFF;
		return (byte_t)ret;
	}
	void reset_DIV_register(){
		this->DIV_register = 0;
	}
};
