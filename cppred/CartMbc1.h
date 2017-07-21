#pragma once
#include "StorageController.h"

class Mbc1Cartridge : public StandardCartridge{
protected:

	virtual void init_functions_derived() override;
	virtual void set_ram_functions();
	static byte_t read8_simple(StandardCartridge *, main_integer_t);
	static byte_t read8_switchable_rom_bank(StandardCartridge *, main_integer_t);
	static byte_t read8_switchable_ram_bank(StandardCartridge *, main_integer_t);
	static byte_t read8_small_ram(StandardCartridge *, main_integer_t);
	static byte_t read8_invalid_ram(StandardCartridge *, main_integer_t);
	static void write8_ram_enable(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switch_rom_bank_low(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switch_rom_bank_high_or_ram(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switch_rom_ram_banking_mode(StandardCartridge *, main_integer_t, byte_t);
	static void write8_switchable_ram_bank(StandardCartridge *, main_integer_t, byte_t);
	static void write8_small_ram(StandardCartridge *, main_integer_t, byte_t);
	static void write8_invalid_ram(StandardCartridge *, main_integer_t, byte_t);

	void toggle_ram(bool);
	void toggle_ram_banking(bool);
	main_integer_t compute_rom_offset(main_integer_t address);
	main_integer_t compute_ram_offset(main_integer_t address);
public:
	Mbc1Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
	virtual ~Mbc1Cartridge(){}
	void post_initialization() override;
};
