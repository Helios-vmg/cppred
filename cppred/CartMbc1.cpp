#include "CartMbc1.h"

Mbc1Cartridge::Mbc1Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc):
	StandardCartridge(host, std::move(buffer), cc){
}

void Mbc1Cartridge::init_functions_derived(){
	for (unsigned i = 0; i < 0x40; i++)
		this->read_callbacks[i] = read8_simple;
	for (unsigned i = 0x40; i < 0x80; i++)
		this->read_callbacks[i] = read8_switchable_rom_bank;

	if (this->capabilities.has_ram){
		for (unsigned i = 0; i < 0x20; i++)
			this->write_callbacks[i] = write8_ram_enable;
	}

	for (unsigned i = 0x20; i < 0x40; i++)
		this->write_callbacks[i] = write8_switch_rom_bank_low;
	for (unsigned i = 0x40; i < 0x60; i++)
		this->write_callbacks[i] = write8_switch_rom_bank_high_or_ram;
	for (unsigned i = 0x60; i < 0x80; i++)
		this->write_callbacks[i] = write8_switch_rom_ram_banking_mode;

	this->set_ram_functions();
}

void Mbc1Cartridge::set_ram_functions(){
	if (!this->capabilities.has_ram)
		return;

	if (!this->ram_enabled){
		for (unsigned i = 0xA0; i < 0xC0; i++){
			this->write_callbacks[i] = write8_invalid_ram;
			this->read_callbacks[i] = read8_invalid_ram;
		}
		return;
	}

	if (this->capabilities.ram_size == 1 << 11){
		for (unsigned i = 0xA0; i < 0xA8; i++){
			this->write_callbacks[i] = write8_small_ram;
			this->read_callbacks[i] = read8_small_ram;
		}
		for (unsigned i = 0xA8; i < 0xC0; i++){
			this->write_callbacks[i] = write8_invalid_ram;
			this->read_callbacks[i] = read8_invalid_ram;
		}
	}else{
		for (unsigned i = 0xA0; i < 0xC0; i++){
			this->write_callbacks[i] = write8_switchable_ram_bank;
			this->read_callbacks[i] = read8_switchable_ram_bank;
		}
	}
}

byte_t Mbc1Cartridge::read8_simple(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	return This->data[address];
}

byte_t Mbc1Cartridge::read8_switchable_rom_bank(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	auto offset = This->compute_rom_offset(address);
	return This->data[offset];
}

byte_t Mbc1Cartridge::read8_small_ram(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	return This->ram.read(address & 0x7FFF);
}

byte_t Mbc1Cartridge::read8_switchable_ram_bank(StandardCartridge *sc, main_integer_t address){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	auto offset = This->compute_ram_offset(address);
	return This->ram.read(offset);
}

void Mbc1Cartridge::write8_ram_enable(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	This->toggle_ram((value & 0x0F) == 0x0A);
}

void Mbc1Cartridge::write8_switch_rom_bank_low(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	const decltype(This->current_rom_bank) mask = 0x1F;
	value &= mask;
	This->current_rom_bank &= ~mask;
	This->current_rom_bank |= value & mask;
	This->current_rom_bank %= This->rom_bank_count;
	if (!(This->current_rom_bank & mask))
		This->current_rom_bank++;
}

void Mbc1Cartridge::write8_switch_rom_bank_high_or_ram(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	const decltype(This->current_rom_bank) mask = 3;
	value &= mask;
	This->ram_bank_bits_copy = value;
	This->toggle_ram_banking(This->ram_banking_mode);
}

void Mbc1Cartridge::write8_switch_rom_ram_banking_mode(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	This->toggle_ram_banking(!!value);
}

void Mbc1Cartridge::write8_switchable_ram_bank(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	auto offset = This->compute_ram_offset(address);
	This->ram.write(offset, value);
}

byte_t Mbc1Cartridge::read8_invalid_ram(StandardCartridge *, main_integer_t){
	throw GenericException("Attempt to read from invalid cartridge RAM.");
}

void Mbc1Cartridge::write8_small_ram(StandardCartridge *sc, main_integer_t address, byte_t value){
	auto This = static_cast<Mbc1Cartridge *>(sc);
	//address -= 0xA000; (Unnecessary due to bitwise AND.)
	auto offset = address & 0x7FF;
	This->ram.write(offset, value);
}

void Mbc1Cartridge::write8_invalid_ram(StandardCartridge *, main_integer_t, byte_t){
	throw GenericException("Attempt to write to invalid cartridge RAM.");
}

main_integer_t Mbc1Cartridge::compute_rom_offset(main_integer_t address){
	return (address & 0x3FFF) | (this->current_rom_bank << 14);
}

main_integer_t Mbc1Cartridge::compute_ram_offset(main_integer_t address){
	address -= 0xA000;
	return (address & 0x1FFF) | (this->current_ram_bank << 13);
}

void Mbc1Cartridge::toggle_ram(bool enable){
	if (!(this->ram_enabled ^ enable))
		return;
	if (this->ram_enabled)
		this->commit_ram();
	this->ram_enabled = enable;
	this->set_ram_functions();
}

void Mbc1Cartridge::toggle_ram_banking(bool enable){
	const decltype(this->current_rom_bank) rom_mask = 0xE0;
	this->ram_banking_mode = enable;
	if (this->ram_banking_mode)
		this->current_ram_bank = this->ram_bank_bits_copy;
	else{
		this->current_rom_bank &= ~rom_mask;
		this->current_rom_bank |= this->ram_bank_bits_copy << 5;
	}
}

void Mbc1Cartridge::post_initialization(){
	StandardCartridge::post_initialization();
	this->load_ram();
}
