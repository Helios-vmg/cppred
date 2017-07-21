#pragma once
#include "CartMbc1.h"

class Mbc3Cartridge : public Mbc1Cartridge{
protected:
	int current_rtc_register = -1;
	int rtc_latch = -1;
	struct RTC{
		byte_t seconds,
			minutes,
			hours,
			day_counter_low,
			day_counter_high;
		bool time_changed = false;
	};
	RTC rtc_registers;
	posix_time_t rtc_start_time = -1;
	posix_time_t rtc_pause_time = -1;
	static const byte_t rtc_8th_day_bit_mask = bit(0);
	static const byte_t rtc_stop_mask = bit(6);
	static const byte_t rtc_overflow_mask = bit(7);

	virtual void init_functions_derived() override;
	virtual void set_ram_functions() override;
	void set_rtc_registers();
	posix_delta_t get_rtc_counter_value_ignoring_pause();
	posix_delta_t get_rtc_counter_value();
	void stop_rtc();
	void resume_rtc();

	static byte_t read8_rtc_register(StandardCartridge *, main_integer_t);
	static void write8_switch_rom_bank(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switch_ram_bank(StandardCartridge *, main_integer_t, byte_t);
	static void write8_latch_rtc_registers(StandardCartridge *, main_integer_t, byte_t);
	static void write8_rtc_register(StandardCartridge *, main_integer_t, byte_t);
public:
	Mbc3Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
	virtual void post_initialization() override;
};
