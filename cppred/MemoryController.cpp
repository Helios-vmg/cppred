#include "MemoryController.h"
#include "GameboyCpu.h"
#include "Gameboy.h"
#include "exceptions.h"
#include <cstdlib>
#include <exception>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>

unsigned char gb_bootstrap_rom[] = {
	0x31,0xFE,0xFF,0xAF,0x21,0xFF,0x9F,0x32,0xCB,0x7C,0x20,0xFB,0x21,0x26,0xFF,0x0E,
	0x11,0x3E,0x80,0x32,0xE2,0x0C,0x3E,0xF3,0xE2,0x32,0x3E,0x77,0x77,0x3E,0xFC,0xE0,
	0x47,0x11,0x04,0x01,0x21,0x10,0x80,0x1A,0xCD,0x95,0x00,0xCD,0x96,0x00,0x13,0x7B,
	0xFE,0x34,0x20,0xF3,0x11,0xD8,0x00,0x06,0x08,0x1A,0x13,0x22,0x23,0x05,0x20,0xF9,
	0x3E,0x19,0xEA,0x10,0x99,0x21,0x2F,0x99,0x0E,0x0C,0x3D,0x28,0x08,0x32,0x0D,0x20,
	0xF9,0x2E,0x0F,0x18,0xF3,0x67,0x3E,0x64,0x57,0xE0,0x42,0x3E,0x91,0xE0,0x40,0x04,
	0x1E,0x02,0x0E,0x0C,0xF0,0x44,0xFE,0x90,0x20,0xFA,0x0D,0x20,0xF7,0x1D,0x20,0xF2,
	0x0E,0x13,0x24,0x7C,0x1E,0x83,0xFE,0x62,0x28,0x06,0x1E,0xC1,0xFE,0x64,0x20,0x06,
	0x7B,0xE2,0x0C,0x3E,0x87,0xE2,0xF0,0x42,0x90,0xE0,0x42,0x15,0x20,0xD2,0x05,0x20,
	0x4F,0x16,0x20,0x18,0xCB,0x4F,0x06,0x04,0xC5,0xCB,0x11,0x17,0xC1,0xCB,0x11,0x17,
	0x05,0x20,0xF5,0x22,0x23,0x22,0x23,0xC9,0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
	0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
	0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
	0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E,0x3C,0x42,0xB9,0xA5,0xB9,0xA5,0x42,0x3C,
	0x21,0x04,0x01,0x11,0xA8,0x00,0x1A,0x13,0xBE,0x20,0xFE,0x23,0x7D,0xFE,0x34,0x20,
	0xF5,0x06,0x19,0x78,0x86,0x23,0x05,0x20,0xFB,0x86,0x20,0xFE,0x3E,0x01,0xE0,0x50
};
const size_t gb_bootstrap_rom_size = sizeof(gb_bootstrap_rom);

const size_t io_function_table_sizes = 0x100;

MemoryController::MemoryController(Gameboy &system, GameboyCpu &cpu):
		system(&system),
		cpu(&cpu),
		display(&system.get_display_controller()),
		sound(&system.get_sound_controller()),
		joypad(&system.get_input_controller()),
		storage(&system.get_storage_controller()),
		fixed_ram(0x1000),
		switchable_ram(0x7000),
		high_ram(0x80),
		io_registers_stor(new store_func_t[io_function_table_sizes]),
		io_registers_load(new load_func_t[io_function_table_sizes]),
		memory_map_store(new store_func_t[0x100]),
		memory_map_load(new load_func_t[0x100]){
#ifdef DEBUG_MEMORY_STORES
	this->last_store_at.reset(new std::uint32_t[0x10000]);
	this->last_store_at_clock.reset(new std::uint64_t[0x10000]);
	std::fill(this->last_store_at.get(), this->last_store_at.get() + 0x10000, 0xFFFF);
	std::fill(this->last_store_at_clock.get(), this->last_store_at_clock.get() + 0x10000, std::numeric_limits<std::uint64_t>::max());
#endif
}

MemoryController::~MemoryController(){
#ifdef IO_REGISTERS_RECORDING
	if (this->operation_mode == MemoryOperationMode::Recording){
		std::ofstream file(this->recording_file_path, std::ios::binary);
		if (file){
			for (auto item : *this->recording)
				file.write((const char *)&item, sizeof(item));
		}
	}
#endif
}

void MemoryController::initialize(){
	this->display->set_memory_controller(*this);
	this->initialize_functions();
}

void MemoryController::initialize_functions(){
	this->initialize_memory_map_functions();
	this->initialize_io_register_functions();
}

#define MEMORY_RANGE_FOR(end) \
	old = accum; \
	accum = end; \
	for (unsigned i = old; i < end; i++)

void MemoryController::initialize_memory_map_functions(){
	unsigned accum = 0, old;
	//[0x0000; 0x8000)
	MEMORY_RANGE_FOR(0x80){
		this->memory_map_load[i] = &MemoryController::read_storage;
		this->memory_map_store[i] = &MemoryController::write_storage;
	}
	//[0x8000; 0xA000)
	MEMORY_RANGE_FOR(0xA0){
		this->memory_map_load[i] = &MemoryController::read_vram;
		this->memory_map_store[i] = &MemoryController::write_vram;
	}
	//[0xA000; 0xC000)
	MEMORY_RANGE_FOR(0xC0){
		this->memory_map_load[i] = &MemoryController::read_storage_ram;
		this->memory_map_store[i] = &MemoryController::write_storage_ram;
	}
	//[0xC000; 0xD000)
	MEMORY_RANGE_FOR(0xD0){
		this->memory_map_load[i] = &MemoryController::read_fixed_ram;
		this->memory_map_store[i] = &MemoryController::write_fixed_ram;
	}
	//[0xD000; 0xE000)
	MEMORY_RANGE_FOR(0xE0){
		this->memory_map_load[i] = &MemoryController::read_switchable_ram;
		this->memory_map_store[i] = &MemoryController::write_switchable_ram;
	}
	//[0xE000; 0xF000)
	MEMORY_RANGE_FOR(0xF0){
		this->memory_map_load[i] = &MemoryController::read_ram_mirror1;
		this->memory_map_store[i] = &MemoryController::write_ram_mirror1;
	}
	//[0xF000; 0xFE00)
	MEMORY_RANGE_FOR(0xFE){
		this->memory_map_load[i] = &MemoryController::read_ram_mirror2;
		this->memory_map_store[i] = &MemoryController::write_ram_mirror2;
	}
	//[0xFE00; 0xFF00)
	this->toggle_oam_access(true);
	//[0xFF00; 0xFFFF]
	this->memory_map_load[0xFF] = &MemoryController::read_io_registers_and_high_ram;
	this->memory_map_store[0xFF] = &MemoryController::write_io_registers_and_high_ram;
}

void MemoryController::initialize_io_register_functions(){
	std::fill(this->io_registers_stor.get() + 0x00, this->io_registers_stor.get() + 0x4C, &MemoryController::store_no_io);
	std::fill(this->io_registers_stor.get() + 0x4C, this->io_registers_stor.get() + 0x80, &MemoryController::store_no_io);
	std::fill(this->io_registers_stor.get() + 0x80, this->io_registers_stor.get() + io_function_table_sizes, &MemoryController::store_high_ram);

	std::fill(this->io_registers_load.get() + 0x00, this->io_registers_load.get() + 0x4C, &MemoryController::load_no_io);
	std::fill(this->io_registers_load.get() + 0x4C, this->io_registers_load.get() + 0x80, &MemoryController::load_no_io);
	std::fill(this->io_registers_load.get() + 0x80, this->io_registers_load.get() + io_function_table_sizes, &MemoryController::load_high_ram);

	this->io_registers_stor[0x00] = &MemoryController::store_P1;
	this->io_registers_load[0x00] = &MemoryController::load_P1;
	//Serial I/O (SB)
	//this->io_registers_stor[0x01] = &MemoryController::store_not_implemented;
	//this->io_registers_load[0x01] = &MemoryController::load_not_implemented;
	//Serial I/O control (SC)
	//this->io_registers_stor[0x02] = &MemoryController::store_not_implemented;
	//this->io_registers_load[0x02] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x03] = &MemoryController::store_not_implemented;
	this->io_registers_load[0x03] = &MemoryController::load_not_implemented;
	this->io_registers_stor[0x04] = &MemoryController::store_DIV;
	this->io_registers_load[0x04] = &MemoryController::load_DIV;
	this->io_registers_stor[0x05] = &MemoryController::store_TIMA;
	this->io_registers_load[0x05] = &MemoryController::load_TIMA;
	this->io_registers_stor[0x06] = &MemoryController::store_TMA;
	this->io_registers_load[0x06] = &MemoryController::load_TMA;
	this->io_registers_stor[0x07] = &MemoryController::store_TAC;
	this->io_registers_load[0x07] = &MemoryController::load_TAC;
	for (unsigned i = 0x08; i < 0x0f; i++){
		this->io_registers_stor[i] = &MemoryController::store_nothing;
		this->io_registers_load[i] = &MemoryController::load_nothing;
	}
	this->io_registers_stor[0x0f] = &MemoryController::store_IF;
	this->io_registers_load[0x0f] = &MemoryController::load_IF;

	//Audio:
	this->io_registers_stor[0x10] = &MemoryController::store_NR10;
	this->io_registers_load[0x10] = &MemoryController::load_NR10;
	this->io_registers_stor[0x11] = &MemoryController::store_NR11;
	this->io_registers_load[0x11] = &MemoryController::load_NR11;
	this->io_registers_stor[0x12] = &MemoryController::store_NR12;
	this->io_registers_load[0x12] = &MemoryController::load_NR12;
	this->io_registers_stor[0x13] = &MemoryController::store_NR13;
	this->io_registers_load[0x13] = &MemoryController::load_NR13;
	this->io_registers_stor[0x14] = &MemoryController::store_NR14;
	this->io_registers_load[0x14] = &MemoryController::load_NR14;

	//Unused:
	//this->io_registers_stor[0x15] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x15] = &MemoryController::load_not_implemented;

	//Audio:
	this->io_registers_stor[0x16] = &MemoryController::store_NR21;
	this->io_registers_load[0x16] = &MemoryController::load_NR21;
	this->io_registers_stor[0x17] = &MemoryController::store_NR22;
	this->io_registers_load[0x17] = &MemoryController::load_NR22;
	this->io_registers_stor[0x18] = &MemoryController::store_NR23;
	this->io_registers_load[0x18] = &MemoryController::load_NR23;
	this->io_registers_stor[0x19] = &MemoryController::store_NR24;
	this->io_registers_load[0x19] = &MemoryController::load_NR24;
	this->io_registers_stor[0x1a] = &MemoryController::store_NR30;
	this->io_registers_load[0x1a] = &MemoryController::load_NR30;
	this->io_registers_stor[0x1b] = &MemoryController::store_NR31;
	this->io_registers_load[0x1b] = &MemoryController::load_NR31;
	this->io_registers_stor[0x1c] = &MemoryController::store_NR32;
	this->io_registers_load[0x1c] = &MemoryController::load_NR32;
	this->io_registers_stor[0x1d] = &MemoryController::store_NR33;
	this->io_registers_load[0x1d] = &MemoryController::load_NR33;
	this->io_registers_stor[0x1e] = &MemoryController::store_NR34;
	this->io_registers_load[0x1e] = &MemoryController::load_NR30;

	//Unused:
	//this->io_registers_stor[0x1f] = &MemoryController::stor_not_implemented;
	//this->io_registers_load[0x1f] = &MemoryController::load_not_implemented;

	//Audio:
	this->io_registers_stor[0x20] = &MemoryController::store_NR41;
	this->io_registers_load[0x20] = &MemoryController::load_NR41;
	this->io_registers_stor[0x21] = &MemoryController::store_NR42;
	this->io_registers_load[0x21] = &MemoryController::load_NR42;
	this->io_registers_stor[0x22] = &MemoryController::store_NR43;
	this->io_registers_load[0x22] = &MemoryController::load_NR43;
	this->io_registers_stor[0x23] = &MemoryController::store_NR44;
	this->io_registers_load[0x23] = &MemoryController::load_NR44;

	this->io_registers_stor[0x24] = &MemoryController::store_NR50;
	this->io_registers_load[0x24] = &MemoryController::load_NR50;
	this->io_registers_stor[0x25] = &MemoryController::store_NR51;
	this->io_registers_load[0x25] = &MemoryController::load_NR51;
	this->io_registers_stor[0x26] = &MemoryController::store_NR52;
	this->io_registers_load[0x26] = &MemoryController::load_NR52;

	for (unsigned i = 0x27; i < 0x30; i++){
		this->io_registers_stor[i] = &MemoryController::store_nothing;
		this->io_registers_load[i] = &MemoryController::load_nothing;
	}

	//Audio:
	for (unsigned i = 0x30; i < 0x40; i++){
		this->io_registers_stor[i] = &MemoryController::store_WAVE;
		this->io_registers_load[i] = &MemoryController::load_WAVE;
	}

	this->io_registers_stor[0x40] = &MemoryController::store_LCDC;
	this->io_registers_load[0x40] = &MemoryController::load_LCDC;
	this->io_registers_stor[0x41] = &MemoryController::store_STAT;
	this->io_registers_load[0x41] = &MemoryController::load_STAT;
	this->io_registers_stor[0x42] = &MemoryController::store_SCY;
	this->io_registers_load[0x42] = &MemoryController::load_SCY;
	this->io_registers_stor[0x43] = &MemoryController::store_SCX;
	this->io_registers_load[0x43] = &MemoryController::load_SCX;
	this->io_registers_stor[0x44] = &MemoryController::store_LY;
	this->io_registers_load[0x44] = &MemoryController::load_LY;
	this->io_registers_stor[0x45] = &MemoryController::store_LYC;
	this->io_registers_load[0x45] = &MemoryController::load_LYC;
	this->io_registers_stor[0x46] = &MemoryController::store_DMA;
	//this->io_registers_load[0x46] = &MemoryController::load_DMA;
	this->io_registers_stor[0x47] = &MemoryController::store_BGP;
	this->io_registers_load[0x47] = &MemoryController::load_BGP;
	this->io_registers_stor[0x48] = &MemoryController::store_OBP0;
	this->io_registers_load[0x48] = &MemoryController::load_OBP0;
	this->io_registers_stor[0x49] = &MemoryController::store_OBP1;
	this->io_registers_load[0x49] = &MemoryController::load_OBP1;
	this->io_registers_stor[0x4a] = &MemoryController::store_WY;
	this->io_registers_load[0x4a] = &MemoryController::load_WY;
	this->io_registers_stor[0x4b] = &MemoryController::store_WX;
	this->io_registers_load[0x4b] = &MemoryController::load_WX;

	this->io_registers_stor[0x50] = &MemoryController::store_bootstrap_rom_enable;
	this->io_registers_load[0x50] = &MemoryController::load_bootstrap_rom_enable;

	this->io_registers_stor[0xFF] = &MemoryController::store_interrupt_enable;
	this->io_registers_load[0xFF] = &MemoryController::load_interrupt_enable;
}

byte_t MemoryController::read_storage(main_integer_t address) const{
	return this->storage->read8(address);
}

void MemoryController::write_storage(main_integer_t address, byte_t value){
	this->storage->write8(address, value);
}

byte_t MemoryController::read_storage_ram(main_integer_t address) const{
	return this->read_storage(address);
}

void MemoryController::write_storage_ram(main_integer_t address, byte_t value){
	return this->write_storage(address, value);
}

byte_t MemoryController::read_ram_mirror1(main_integer_t address) const{
	return this->read_fixed_ram(address - 0x2000);
}

void MemoryController::write_ram_mirror1(main_integer_t address, byte_t value){
	this->write_fixed_ram(address - 0x2000, value);
}

byte_t MemoryController::read_ram_mirror2(main_integer_t address) const{
	return this->read_switchable_ram(address - 0x2000);
}

void MemoryController::write_ram_mirror2(main_integer_t address, byte_t value){
	this->write_switchable_ram(address - 0x2000, value);
}

#ifdef IO_REGISTERS_RECORDING
bool recording_exhausted = false;
#endif

byte_t MemoryController::read_io_registers_and_high_ram(main_integer_t address) const{
	std::uint8_t truncated_address = address & 0xFF;
	auto fp = this->io_registers_load[truncated_address];
	auto ret = (this->*fp)(address);
#ifdef IO_REGISTERS_RECORDING
	if (!(truncated_address & 0x80)){
		std::uint64_t clock_value = this->system->get_system_clock().get_clock_value();
		RecordingInstant front;
		switch (this->operation_mode){
			case MemoryOperationMode::Default:
				break;
			case MemoryOperationMode::Recording:
				this->recording->push_back(RecordingInstant{ clock_value & 0xFFFFFFFF, (clock_value >> 32) & 0xFFFF, truncated_address, ret });
				break;
			case MemoryOperationMode::Playback:
				if (!this->recording->size()){
					if (!recording_exhausted){
						std::cout << "IO recording has run out!.\n";
						recording_exhausted = true;
					}
					break;
				}
				front = this->recording->front();
				if (front.get_clock_value() == clock_value && truncated_address == front.io_register){
					ret = front.read_value;
					this->recording->pop_front();
				}
				break;
		}
	}
#endif
	return ret;
}

void MemoryController::write_io_registers_and_high_ram(main_integer_t address, byte_t value){
	auto fp = this->io_registers_stor[address & 0xFF];
	if (fp)
		(this->*fp)(address, value);
}

byte_t MemoryController::read_dmg_bootstrap(main_integer_t address) const{
	return gb_bootstrap_rom[address];
}

byte_t MemoryController::read_vram(main_integer_t address) const{
	if (!this->vram_enabled)
		return 0xFF;
	return this->display->access_vram(address);
}

void MemoryController::write_vram(main_integer_t address, byte_t value){
	if (!this->vram_enabled)
		return;
	this->display->access_vram(address) = value;
}

byte_t MemoryController::read_fixed_ram(main_integer_t address) const{
	return this->fixed_ram.access(address);
}

void MemoryController::write_fixed_ram(main_integer_t address, byte_t value){
	this->fixed_ram.access(address) = value;
}

byte_t MemoryController::read_switchable_ram(main_integer_t address) const{
	return this->switchable_ram.access(address + (this->selected_ram_bank << 12));
}

void MemoryController::write_switchable_ram(main_integer_t address, byte_t value){
	this->switchable_ram.access(address + (this->selected_ram_bank << 12)) = value;
}

byte_t MemoryController::read_oam(main_integer_t address) const{
	if (address >= 0xFEA0)
		return 0;

	return this->display->access_oam(address);
}

byte_t MemoryController::read_disabled_oam(main_integer_t address) const{
	return (address < 0xFEA0) * 0xFF;
}

void MemoryController::write_oam(main_integer_t address, byte_t value){
	if (address >= 0xFEA0)
		return;

	this->display->access_oam(address) = value;
}

void MemoryController::write_disabled_oam(main_integer_t address, byte_t value){
	this->store_nothing(address, value);
}

void MemoryController::store_nothing(main_integer_t, byte_t){
}

byte_t MemoryController::load_nothing(main_integer_t) const{
	return 0xFF;
}

void MemoryController::store_not_implemented(main_integer_t, byte_t){
	throw NotImplementedException();
}

byte_t MemoryController::load_not_implemented(main_integer_t) const{
	throw NotImplementedException();
}

void MemoryController::store_no_io(main_integer_t, byte_t){
}

byte_t MemoryController::load_no_io(main_integer_t) const{
	return 0xFF;
}

void MemoryController::store_bootstrap_rom_enable(main_integer_t, byte_t value){
	this->toggle_boostrap_rom(!value);
}

byte_t MemoryController::load_bootstrap_rom_enable(main_integer_t) const{
	return 0xFF;
}

void MemoryController::store_high_ram(main_integer_t address, byte_t value){
	this->high_ram.access(address) = value;
}

byte_t MemoryController::load_high_ram(main_integer_t address) const{
	return this->high_ram.access(address);
}

void MemoryController::store_interrupt_enable(main_integer_t, byte_t value){
	this->cpu->set_interrupt_enable_flag(value);
}

byte_t MemoryController::load_interrupt_enable(main_integer_t) const{
	return this->cpu->get_interrupt_enable_flag();
}

byte_t MemoryController::load_STAT(main_integer_t) const{
	return this->display->get_status();
}

void MemoryController::store_STAT(main_integer_t, byte_t b){
	this->display->set_status(b);
}

byte_t MemoryController::load_LY(main_integer_t) const{
	return this->display->get_y_coordinate();
}

void MemoryController::store_LY(main_integer_t, byte_t b){
	this->display->set_y_coordinate_compare(b);
}

byte_t MemoryController::load_LYC(main_integer_t) const{
	return this->display->get_y_coordinate_compare();
}

void MemoryController::store_LYC(main_integer_t, byte_t b){
	this->display->set_y_coordinate_compare(b);
}

byte_t MemoryController::load_WY(main_integer_t) const{
	return this->display->get_window_y_position();
}

void MemoryController::store_WY(main_integer_t, byte_t b){
	this->display->set_window_y_position(b);
}

byte_t MemoryController::load_WX(main_integer_t) const{
	return this->display->get_window_x_position();
}

void MemoryController::store_WX(main_integer_t, byte_t b){
	this->display->set_window_x_position(b);
}

byte_t MemoryController::load_BGP(main_integer_t) const{
	return this->display->get_background_palette();
}

void MemoryController::store_BGP(main_integer_t, byte_t b){
	this->display->set_background_palette(b);
}

byte_t MemoryController::load_SCY(main_integer_t) const{
	return this->display->get_scroll_y();
}

void MemoryController::store_SCY(main_integer_t, byte_t b){
	this->display->set_scroll_y(b);
}

byte_t MemoryController::load_SCX(main_integer_t) const{
	return this->display->get_scroll_x();
}

void MemoryController::store_SCX(main_integer_t, byte_t b){
	this->display->set_scroll_x(b);
}

byte_t MemoryController::load_LCDC(main_integer_t) const{
	return this->display->get_lcd_control();
}

void MemoryController::store_LCDC(main_integer_t, byte_t b){
	this->display->set_lcd_control(b);
}

byte_t MemoryController::load_IF(main_integer_t) const{
	return this->cpu->get_interrupt_flag();
}

void MemoryController::store_IF(main_integer_t, byte_t b){
	this->cpu->set_interrupt_flag(b);
}

byte_t MemoryController::load_P1(main_integer_t) const{
	return this->joypad->get_requested_input_state();
}

void MemoryController::store_P1(main_integer_t, byte_t b){
	this->joypad->request_input_state(b);
}

byte_t MemoryController::load_OBP0(main_integer_t) const{
	return this->display->get_obj0_palette();
}

void MemoryController::store_OBP0(main_integer_t, byte_t b){
	this->display->set_obj0_palette(b);
}

byte_t MemoryController::load_OBP1(main_integer_t) const{
	return this->display->get_obj1_palette();
}

void MemoryController::store_OBP1(main_integer_t, byte_t b){
	this->display->set_obj1_palette(b);
}

byte_t MemoryController::load_DMA(main_integer_t) const{
	return 0;
}

void MemoryController::store_DMA(main_integer_t, byte_t b){
	this->cpu->begin_dmg_dma_transfer(b);
}

byte_t MemoryController::load_DIV(main_integer_t) const{
	return this->system->get_system_clock().get_DIV_register();
}

void MemoryController::store_DIV(main_integer_t, byte_t b){
	this->system->get_system_clock().reset_DIV_register();
}

byte_t MemoryController::load_TIMA(main_integer_t) const{
	return this->system->get_system_clock().get_TIMA_register();
}

void MemoryController::store_TIMA(main_integer_t, byte_t b){
	this->system->get_system_clock().set_TIMA_register(b);
}

byte_t MemoryController::load_TMA(main_integer_t) const{
	return this->system->get_system_clock().get_TMA_register();
}

void MemoryController::store_TMA(main_integer_t, byte_t b){
	this->system->get_system_clock().set_TMA_register(b);
}

byte_t MemoryController::load_TAC(main_integer_t) const{
	return this->system->get_system_clock().get_TAC_register();
}

void MemoryController::store_TAC(main_integer_t, byte_t b){
	this->system->get_system_clock().set_TAC_register(b);
}

byte_t MemoryController::load_NR10(main_integer_t) const{
	return this->sound->square1.get_register0();
}

void MemoryController::store_NR10(main_integer_t, byte_t b){
	this->sound->square1.set_register0(b);
}

byte_t MemoryController::load_NR11(main_integer_t) const{
	return this->sound->square1.get_register1();
}

void MemoryController::store_NR11(main_integer_t, byte_t b){
	this->sound->square1.set_register1(b);
}

byte_t MemoryController::load_NR12(main_integer_t) const{
	return this->sound->square1.get_register2();
}

void MemoryController::store_NR12(main_integer_t, byte_t b){
	this->sound->square1.set_register2(b);
}

byte_t MemoryController::load_NR13(main_integer_t) const{
	return this->sound->square1.get_register3();
}

void MemoryController::store_NR13(main_integer_t, byte_t b){
	this->sound->square1.set_register3(b);
}

byte_t MemoryController::load_NR14(main_integer_t) const{
	return this->sound->square1.get_register4();
}

void MemoryController::store_NR14(main_integer_t, byte_t b){
	this->sound->square1.set_register4(b);
}

byte_t MemoryController::load_NR21(main_integer_t) const{
	return this->sound->square2.get_register1();
}

void MemoryController::store_NR21(main_integer_t, byte_t b){
	this->sound->square2.set_register1(b);
}

byte_t MemoryController::load_NR22(main_integer_t) const{
	return this->sound->square2.get_register2();
}

void MemoryController::store_NR22(main_integer_t, byte_t b){
	this->sound->square2.set_register2(b);
}

byte_t MemoryController::load_NR23(main_integer_t) const{
	return this->sound->square2.get_register3();
}

void MemoryController::store_NR23(main_integer_t, byte_t b){
	this->sound->square2.set_register3(b);
}

byte_t MemoryController::load_NR24(main_integer_t) const{
	return this->sound->square2.get_register4();
}

void MemoryController::store_NR24(main_integer_t, byte_t b){
	this->sound->square2.set_register4(b);
}



byte_t MemoryController::load_NR30(main_integer_t) const{
	return this->sound->wave.get_register0();
}

void MemoryController::store_NR30(main_integer_t, byte_t b){
	this->sound->wave.set_register0(b);
}

byte_t MemoryController::load_NR31(main_integer_t) const{
	return this->sound->wave.get_register1();
}

void MemoryController::store_NR31(main_integer_t, byte_t b){
	this->sound->wave.set_register1(b);
}

byte_t MemoryController::load_NR32(main_integer_t) const{
	return this->sound->wave.get_register2();
}

void MemoryController::store_NR32(main_integer_t, byte_t b){
	this->sound->wave.set_register2(b);
}

byte_t MemoryController::load_NR33(main_integer_t) const{
	return this->sound->wave.get_register3();
}

void MemoryController::store_NR33(main_integer_t, byte_t b){
	this->sound->wave.set_register3(b);
}

byte_t MemoryController::load_NR34(main_integer_t) const{
	return this->sound->wave.get_register4();
}

void MemoryController::store_NR34(main_integer_t, byte_t b){
	this->sound->wave.set_register4(b);
}



byte_t MemoryController::load_NR41(main_integer_t) const{
	return this->sound->noise.get_register1();
}

void MemoryController::store_NR41(main_integer_t, byte_t b){
	this->sound->noise.set_register1(b);
}

byte_t MemoryController::load_NR42(main_integer_t) const{
	return this->sound->noise.get_register2();
}

void MemoryController::store_NR42(main_integer_t, byte_t b){
	this->sound->noise.set_register2(b);
}

byte_t MemoryController::load_NR43(main_integer_t) const{
	return this->sound->noise.get_register3();
}

void MemoryController::store_NR43(main_integer_t, byte_t b){
	this->sound->noise.set_register3(b);
}

byte_t MemoryController::load_NR44(main_integer_t) const{
	return this->sound->noise.get_register4();
}

void MemoryController::store_NR44(main_integer_t, byte_t b){
	this->sound->noise.set_register4(b);
}

byte_t MemoryController::load_NR50(main_integer_t) const{
	return this->sound->get_NR50();
}

void MemoryController::store_NR50(main_integer_t, byte_t b){
	this->sound->set_NR50(b);
}

byte_t MemoryController::load_NR51(main_integer_t) const{
	return this->sound->get_NR51();
}

void MemoryController::store_NR51(main_integer_t, byte_t b){
	this->sound->set_NR51(b);
}

byte_t MemoryController::load_NR52(main_integer_t) const{
	return this->sound->get_NR52();
}

void MemoryController::store_NR52(main_integer_t, byte_t b){
	this->sound->set_NR52(b);
}

byte_t MemoryController::load_WAVE(main_integer_t) const{
	return 0xFF;
}

void MemoryController::store_WAVE(main_integer_t address, byte_t b){
	this->sound->wave.set_wave_table((unsigned)address - 0xFF30, b);
}

main_integer_t MemoryController::load8(main_integer_t address) const{
	address &= 0xFFFF;
	auto fp = this->memory_map_load[address >> 8];
	return (this->*fp)(address);
}

main_integer_t MemoryController::load8_io(main_integer_t offset) const{
	return this->read_io_registers_and_high_ram(0xFF00 | offset);
}

void MemoryController::store8(main_integer_t address, main_integer_t value){
	address &= 0xFFFF;
#ifdef DEBUG_MEMORY_STORES
	this->last_store_at[address] = this->cpu->get_full_pc();
	this->last_store_at_clock[address] = this->system->get_system_clock().get_clock_value();
#endif
	auto fp = this->memory_map_store[address >> 8];
	(this->*fp)(address, (byte_t)value);
}

void MemoryController::store8_io(main_integer_t offset, main_integer_t value){
#ifdef DEBUG_MEMORY_STORES
	auto address = offset | 0xFF00;
	this->last_store_at[address] = (std::uint16_t)this->cpu->get_current_pc();
	this->last_store_at_clock[address] = this->system->get_system_clock().get_clock_value();
#endif
	this->write_io_registers_and_high_ram(0xFF00 | offset, (byte_t)value);
}

main_integer_t MemoryController::load16(main_integer_t address) const{
	return (this->load8(address + 1) << 8) | this->load8(address);
}

void MemoryController::store16(main_integer_t address, main_integer_t value){
	this->store8(address + 1, value >> 8);
	this->store8(address, value);
}

void MemoryController::copy_memory_force(main_integer_t src, main_integer_t dst, size_t length){
	if (src == dst)
		return;
	if (dst > src && dst < src + length || src > dst && src < dst + length)
		throw GenericException("Invalid memory transfer: source and destination overlap.");

	auto oam = this->get_oam_access_enabled();
	auto vram = this->get_vram_access_enabled();
	//auto palette = this->get_palette_access_enabled()
	this->toggle_oam_access(true);
	this->toggle_vram_access(true);
	//this->toggle_palette_access(true);

	for (size_t i = 0; i < length; i++)
		this->store8(dst + i, this->load8(src + i));

	this->toggle_oam_access(oam);
	this->toggle_vram_access(vram);
	//this->toggle_palette_access(palette);
}

void MemoryController::toggle_boostrap_rom(bool on){
	if (on)
		this->memory_map_load[0x00] = &MemoryController::read_dmg_bootstrap;
	else
		this->memory_map_load[0x00] = &MemoryController::read_storage;
}

bool MemoryController::get_boostrap_enabled() const{
	return this->memory_map_load[0x00] == &MemoryController::read_dmg_bootstrap;
}

void MemoryController::toggle_oam_access(bool enable){
	if (enable){
		this->memory_map_load[0xFE] = &MemoryController::read_oam;
		this->memory_map_store[0xFE] = &MemoryController::write_oam;
	}else{
		this->memory_map_load[0xFE] = &MemoryController::read_disabled_oam;
		this->memory_map_store[0xFE] = &MemoryController::write_disabled_oam;
	}
}

bool MemoryController::get_oam_access_enabled() const{
	return this->memory_map_load[0xFE] == &MemoryController::read_oam;
}

bool MemoryController::get_vram_access_enabled() const{
	return this->vram_enabled;
}

bool MemoryController::get_palette_access_enabled() const{
	throw NotImplementedException();
	return true;
}

void MemoryController::toggle_vram_access(bool enable){
	this->vram_enabled = enable;
}

void MemoryController::toggle_palette_access(bool enable){
	throw NotImplementedException();
}

#ifdef IO_REGISTERS_RECORDING
void MemoryController::use_recording(const char *path, bool record){
	if (record){
		this->operation_mode = MemoryOperationMode::Recording;
		this->recording_file_path = path;
		this->recording.reset(new decltype(this->recording)::element_type);
		std::cout << "Memory controller is running in recording mode.\n";
	}else{
		std::ifstream file(path, std::ios::binary);
		if (!file){
			std::cerr << "MemoryController::use_recording(): ERROR. File not found.\n"
				"Memory controller is running in default mode.\n";
			return;
		}
		this->recording.reset(new decltype(this->recording)::element_type);
		char buffer[sizeof(RecordingInstant)];
		while (true){
			file.read(buffer, sizeof(buffer));
			if (file.gcount() != sizeof(buffer))
				break;
			this->recording->push_back(*(RecordingInstant *)buffer);
		}
		this->operation_mode = MemoryOperationMode::Playback;
		std::cout << "Memory controller is running in playback mode.\n";
	}
}
#endif
