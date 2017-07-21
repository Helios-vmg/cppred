#pragma once

#include "CommonTypes.h"

class Gameboy;

class SystemClock{
	Gameboy *system;
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
	std::uint32_t TIMA_register = 0;
	std::uint8_t TMA_register = 0;
	std::uint8_t TAC_register = 0;
	std::uint32_t timer_enable_mask = 0;
	std::uint32_t tac_mask = 0;
	std::uint32_t tac_mask_bit = 0;
	std::uint32_t tima_overflow = 0;
	bool last_preincrement_value = false;
	bool trigger_interrupt = false;
	static const std::uint32_t tac_selector[][2];

	void cascade_timer_behavior(std::uint32_t old_tac = 0, std::uint32_t new_tac = 0);
	void cascade_timer_behavior_no_check();
	void handle_tima_overflow_part1();
	void handle_tima_overflow_part2();
public:
	SystemClock(Gameboy &system): system(&system){}

	std::uint64_t get_realtime_clock_value() const{
		return this->realtime_clock;
	}
	double get_realtime_clock_value_seconds() const;
	std::uint64_t get_clock_value() const{
		return this->cpu_clock;
	}
	void advance_clock(std::uint32_t clocks);
	byte_t get_DIV_register() const{
		auto ret = this->DIV_register;
		ret >>= 8;
		ret &= 0xFF;
		return (byte_t)ret;
	}
	void reset_DIV_register(){
		this->DIV_register = 0;
		this->cascade_timer_behavior();
	}
	bool get_trigger_interrupt(){
		auto ret = this->trigger_interrupt;
		this->trigger_interrupt = false;
		return ret;
	}
	void set_TIMA_register(byte_t val){
		this->TIMA_register = val;
		this->tima_overflow = 0;
	}
	byte_t get_TIMA_register() const{
		auto ret = this->TIMA_register;
		ret &= 0xFF;
		return (byte_t)ret;
	}
	void set_TMA_register(byte_t val){
		this->TMA_register = val;
		if (this->tima_overflow){
			this->TIMA_register = this->TMA_register;
			this->tima_overflow = 0;
		}
	}
	byte_t get_TMA_register() const{
		auto ret = this->TMA_register;
		ret &= 0xFF;
		return (byte_t)ret;
	}
	void set_TAC_register(byte_t val);
	byte_t get_TAC_register() const{
		auto ret = this->TAC_register;
		ret &= 0xFF;
		return (byte_t)ret;
	}

};
