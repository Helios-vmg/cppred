#include "SystemClock.h"
#include "Gameboy.h"
#include "GameboyCpu.h"
#include <cassert>
#include <limits>

const std::uint32_t SystemClock::tac_selector[][2] = {
	{ 1 << 9, 9 },
	{ 1 << 3, 3 },
	{ 1 << 5, 5 },
	{ 1 << 7, 7 },
};

double SystemClock::get_realtime_clock_value_seconds() const{
	return (double)this->get_realtime_clock_value() * (1.0 / (double)gb_cpu_frequency);
}

void SystemClock::advance_clock(std::uint32_t clocks){
	assert(!(clocks % 4));
	clocks >>= 2;
	while (clocks--){
		this->realtime_clock += 4;
		this->cpu_clock += 4;
		this->DIV_register += 4;
		if (this->tima_overflow)
			this->handle_tima_overflow_part2();
		else
			this->cascade_timer_behavior_no_check();
	}
}

void SystemClock::cascade_timer_behavior(std::uint32_t old_tac, std::uint32_t new_tac){
	auto changed_bits = old_tac ^ new_tac;

	if ((this->system->get_mode() == GameboyMode::CGB) & (changed_bits == 4))
		// If in Game Boy Color mode and only the timer enable bit has changed, do nothing.
		return;
	this->cascade_timer_behavior_no_check();
}

void SystemClock::cascade_timer_behavior_no_check(){
	auto new_preincrement_value = !!(this->DIV_register & this->tac_mask);
	this->TIMA_register += !!(new_preincrement_value & this->last_preincrement_value & this->timer_enable_mask);
	this->handle_tima_overflow_part1();
	this->last_preincrement_value = new_preincrement_value;
}

void SystemClock::handle_tima_overflow_part1(){
	this->tima_overflow = this->TIMA_register >> 8;
}

void SystemClock::handle_tima_overflow_part2(){
	this->TIMA_register = this->TMA_register;
	this->tima_overflow = 0;
	this->trigger_interrupt = true;
}

void SystemClock::set_TAC_register(byte_t val){
	auto old = this->TAC_register;
	this->TAC_register = val;
	this->tac_mask = this->tac_selector[val & 3][0];
	this->tac_mask_bit = this->tac_selector[val & 3][1];
	this->timer_enable_mask = (this->TAC_register & 4) ? std::numeric_limits<decltype(this->timer_enable_mask)>::max() : 0;
	this->cascade_timer_behavior(old, val);
	if (this->tima_overflow){
		this->TIMA_register = this->TMA_register;
		this->tima_overflow = 0;
	}
}
