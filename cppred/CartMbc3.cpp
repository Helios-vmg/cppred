#include "CartMbc3.h"
#include "HostSystem.h"
#include <cassert>

Mbc3Cartridge::Mbc3Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc):
	Mbc1Cartridge(host, std::move(buffer), cc){
}

void Mbc3Cartridge::init_functions_derived(){
	Mbc1Cartridge::init_functions_derived();
	for (unsigned i = 0x20; i < 0x40; i++)
		this->write_callbacks[i] = write8_switch_rom_bank;
	for (unsigned i = 0x40; i < 0x60; i++)
		this->write_callbacks[i] = write8_switch_ram_bank;
	for (unsigned i = 0x60; i < 0x80; i++)
		this->write_callbacks[i] = write8_latch_rtc_registers;
}

void Mbc3Cartridge::set_ram_functions(){
	Mbc1Cartridge::set_ram_functions();
	if (!this->capabilities.has_ram)
		return;

	if (!this->ram_enabled)
		return;

	if (this->capabilities.ram_size == 1 << 11)
		return;

	if (this->current_rtc_register < 0)
		return;

	for (unsigned i = 0xA0; i < 0xC0; i++){
		this->write_callbacks[i] = write8_switchable_ram_bank;
		this->read_callbacks[i] = read8_switchable_ram_bank;
	}
}

posix_delta_t Mbc3Cartridge::get_rtc_counter_value_ignoring_pause(){
	return this->host->get_datetime_provider()->local_now().to_posix() - this->rtc_start_time;
}

posix_delta_t Mbc3Cartridge::get_rtc_counter_value(){
	if (this->rtc_pause_time >= 0)
		return this->host->get_datetime_provider()->local_now().to_posix() - this->rtc_pause_time;
	return this->get_rtc_counter_value_ignoring_pause();
}

void Mbc3Cartridge::set_rtc_registers(){
	auto rtc = this->get_rtc_counter_value();
	auto days = rtc / 86400;
	rtc %= 86400;
	auto hours = rtc / 3600;
	rtc /= 3600;
	auto minutes = rtc / 60;
	rtc %= 60;
	auto seconds = rtc;

	this->rtc_registers.seconds = (byte_t)seconds;
	this->rtc_registers.minutes = (byte_t)minutes;
	this->rtc_registers.hours = (byte_t)hours;
	this->rtc_registers.day_counter_low = days % 256;
	this->rtc_registers.day_counter_high =
		(check_flag(days, 256) * rtc_8th_day_bit_mask) |
		((rtc_pause_time >= 0) * rtc_8th_day_bit_mask) |
		((days >= 512) * rtc_8th_day_bit_mask);
}

void Mbc3Cartridge::write8_switch_rom_bank(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	This->current_rom_bank = value & 0x7F;
	This->current_rom_bank %= This->rom_bank_count;
	if (!This->current_rom_bank)
		This->current_rom_bank = 1;
}

void Mbc3Cartridge::write8_switch_ram_bank(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	if (value < 4){
		This->current_ram_bank = value;
		This->current_rtc_register = -1;
	}else if (value >= 8 && value < 12)
		This->current_rtc_register = value - 8;
}

void Mbc3Cartridge::write8_latch_rtc_registers(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	if (!This->rtc_latch && value)
		This->set_rtc_registers();
	This->rtc_latch = value;
}

byte_t Mbc3Cartridge::read8_rtc_register(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	switch (This->current_rtc_register){
		case 0x08:
			return This->rtc_registers.seconds;
		case 0x09:
			return This->rtc_registers.minutes;
		case 0x0A:
			return This->rtc_registers.hours;
		case 0x0B:
			return This->rtc_registers.day_counter_low;
		case 0x0C:
			return This->rtc_registers.day_counter_high;
	}
	assert(false);
	return 0;
}

void Mbc3Cartridge::write8_rtc_register(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc3Cartridge *>(sc);
	switch (This->current_rtc_register){
		case 0x08:
			This->rtc_registers.time_changed |= This->rtc_registers.seconds != value;
			This->rtc_registers.seconds = value;
			return;
		case 0x09:
			This->rtc_registers.time_changed |= This->rtc_registers.minutes != value;
			This->rtc_registers.minutes = value;
			return;
		case 0x0A:
			This->rtc_registers.time_changed |= This->rtc_registers.hours != value;
			This->rtc_registers.hours = value;
			return;
		case 0x0B:
			This->rtc_registers.time_changed |= This->rtc_registers.day_counter_low != value;
			This->rtc_registers.day_counter_low = value;
			return;
		case 0x0C:
			break;
		default:
			return;
	}
	auto delta = This->rtc_registers.day_counter_high ^ value;
	This->rtc_registers.time_changed |= delta & rtc_8th_day_bit_mask;
	This->rtc_registers.day_counter_high = value;
	if (delta & rtc_stop_mask){
		if (value & rtc_stop_mask)
			This->stop_rtc();
		else
			This->resume_rtc();
	}
}

void Mbc3Cartridge::stop_rtc(){
	this->rtc_pause_time = this->get_rtc_counter_value_ignoring_pause();
	this->rtc_registers.time_changed = false;
}

void Mbc3Cartridge::resume_rtc(){
	auto now = this->host->get_datetime_provider()->local_now().to_posix();
	auto pause_time = this->rtc_pause_time;
	this->rtc_pause_time = -1;
	if (!this->rtc_registers.time_changed){
		this->rtc_start_time += now - pause_time;
		return;
	}
	auto days = check_flag(this->rtc_registers.day_counter_high, rtc_8th_day_bit_mask) * 256 + this->rtc_registers.day_counter_low;
	auto h = this->rtc_registers.hours;
	auto m = this->rtc_registers.minutes;
	auto s = this->rtc_registers.seconds;
	this->rtc_start_time = now - (days * 86400 + h * 3600 + m * 60 + s);
	this->host->save_rtc(*this, this->rtc_start_time);
}

void Mbc3Cartridge::post_initialization(){
	Mbc1Cartridge::post_initialization();
	this->rtc_start_time = this->host->load_rtc(*this);
	if (this->rtc_start_time < 0)
		this->rtc_start_time = 0;
}
