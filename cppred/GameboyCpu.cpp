#include "GameboyCpu.h"
#include "Gameboy.h"
#include "exceptions.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cassert>

GameboyCpu::GameboyCpu(Gameboy &system):
		registers(*this),
		memory_controller(*this->system, *this),
		system(&system){
}

GameboyCpu::~GameboyCpu(){
#ifdef GATHER_INSTRUCTION_STATISTICS
	std::vector<std::pair<unsigned, unsigned>> histogram;
	for (auto &kv : this->instruction_histogram){
		if (kv.first == 0xCB)
			continue;
		histogram.push_back(kv);
	}
	std::sort(histogram.begin(), histogram.end(), [](const auto &a, const auto &b){ return a.second > b.second; });
	for (auto &kv : histogram){
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << kv.first << '\t' << std::dec << kv.second << std::endl;
	}
#endif
}

void GameboyCpu::initialize(){
	this->memory_controller.initialize();
	this->initialize_opcode_tables();
	this->memory_controller.toggle_boostrap_rom(true);
}

void GameboyCpu::take_time(std::uint32_t cycles){
	this->system->get_system_clock().advance_clock(cycles);
}

void GameboyCpu::interrupt_toggle(bool enable){
	this->interrupts_enabled = enable;
}

void GameboyCpu::schedule_interrupt_enable(){
	this->interrupt_enable_scheduled = true;
}

void GameboyCpu::stop(){
	throw NotImplementedException();
}

void GameboyCpu::halt(){
	//If HALT immediately follows an EI instruction, enable interrupts, roll back
	//PC to the start of HALT, and allow any pending interrupts to be handled.
	//Then HALT will be allowed a second chance to execute.
	if (this->interrupt_enable_scheduled){
		this->interrupt_enable_scheduled = false;
		this->interrupt_toggle(true);
		this->registers.pc() = (std::uint16_t)this->current_pc;
		return;
	}

	if (!this->interrupts_enabled || !(this->interrupt_enable_flag & all_interrupts_mask)){
		if (this->system->get_mode() == GameboyMode::DMG)
			this->dmg_halt_bug = true;
		else
			this->take_time(4);
		return;
	}
	this->halted = true;
}

void GameboyCpu::abort(){
	throw GenericException("Gameboy program executed an illegal operation.");
}

byte_t GameboyCpu::load_pc_and_increment(){
	return (byte_t)this->memory_controller.load8(this->registers.pc()++);
}

byte_t GameboyCpu::load_pc(){
	return (byte_t)this->memory_controller.load8(this->registers.pc());
}

#define BREAKPOINT(x) if (this->current_pc == x) __debugbreak()

void GameboyCpu::run_one_instruction(){
	if (this->attempt_to_handle_interrupts())
		this->halted = false;

	bool enable_interrupts = this->interrupt_enable_scheduled;
	this->interrupt_enable_scheduled = false;

	assert(!(enable_interrupts && this->halted));

	if (this->halted){
		this->take_time(4);
	}else{
		this->current_pc = this->registers.pc();
		auto rom_bank = this->system->get_storage_controller().get_current_rom_bank();
		this->full_pc = (std::uint32_t)this->current_pc | (rom_bank < 0 ? 0 : (rom_bank << 16));

		byte_t opcode;
		if (!this->dmg_halt_bug)
			opcode = this->load_pc_and_increment();
		else{
			opcode = this->load_pc();
			this->dmg_halt_bug = false;
		}

		auto function_pointer = this->opcode_table[opcode];
#ifdef GATHER_INSTRUCTION_STATISTICS
		this->instruction_histogram[opcode]++;
#endif

		(this->*function_pointer)();
		this->total_instructions++;
	}

	if (enable_interrupts)
		this->interrupt_toggle(true);

	this->check_timer();
	this->perform_dmg_dma();
}

main_integer_t GameboyCpu::decimal_adjust(main_integer_t value){
	value &= 0xFF;
	if (!this->registers.get(Flags::Subtract)){
		if (this->registers.get(Flags::HalfCarry) || (value & 0x0F) > 9)
			value += 6;

		if (this->registers.get(Flags::Carry) || value > 0x9F)
			value += 0x60;
	}else{
		if (this->registers.get(Flags::HalfCarry))
			value = (value - 6) & 0xFF;

		if (this->registers.get(Flags::Carry))
			value -= 0x60;
	}
	return value;
}

void GameboyCpu::vblank_irq(){
	this->interrupt_flag |= (1 << this->vblank_flag_bit);
}

void GameboyCpu::lcd_stat_irq(){
	this->interrupt_flag |= (1 << this->lcd_stat_flag_bit);
}

void GameboyCpu::joystick_irq(){
	this->interrupt_flag |= (1 << this->joypad_flag_bit);
}

bool GameboyCpu::attempt_to_handle_interrupts(){
	if (!this->interrupts_enabled)
		return false;

	auto interrupts_triggered = this->interrupt_flag;
	auto interrupts_enabled = this->interrupt_enable_flag;
	auto combined = interrupts_triggered & interrupts_enabled & all_interrupts_mask;
	if (!combined)
		return false;

	for (int i = 0; i < 5; i++){
		auto mask = 1 << i;
		if (!(combined & mask))
			continue;
		main_integer_t stack_pointer = this->registers.sp();
		main_integer_t new_stack_pointer = stack_pointer - 2;
		this->registers.sp() = (std::uint16_t)new_stack_pointer;
		main_integer_t program_counter = this->registers.pc();
		this->memory_controller.store16(new_stack_pointer, program_counter);
		this->registers.pc() = (std::uint16_t)(0x0040 + i * 8);
		this->interrupt_flag &= ~mask;
		this->interrupt_toggle(false);
		this->take_time(4 * 5);
		return true;
	}
	return false;
}

byte_t GameboyCpu::get_interrupt_flag() const{
	return this->interrupt_flag;
}

void GameboyCpu::set_interrupt_flag(byte_t b){
	this->interrupt_flag = b;
}

byte_t GameboyCpu::get_interrupt_enable_flag() const{
	return this->interrupt_enable_flag;
}

void GameboyCpu::set_interrupt_enable_flag(byte_t b){
	this->interrupt_enable_flag = b;
}

void GameboyCpu::begin_dmg_dma_transfer(byte_t position){
	this->dma_scheduled = position;
}

void GameboyCpu::perform_dmg_dma(){
	if (this->dma_scheduled < 0)
		return;
	this->memory_controller.copy_memory_force(this->dma_scheduled << 8, 0xFE00, 0xA0);
	this->dma_scheduled = -1;
	this->last_dma_at = this->system->get_system_clock().get_clock_value();
}

void GameboyCpu::check_timer(){
	if (this->system->get_system_clock().get_trigger_interrupt())
		this->interrupt_flag |= this->timer_mask;
}

std::uint64_t GameboyCpu::get_clock() const{
	return this->system->get_system_clock().get_clock_value();
}

void GameboyCpu::opcode_cb(){
	byte_t opcode = this->load_pc_and_increment();
	auto function_pointer = this->opcode_table_cb[opcode];
#ifdef GATHER_INSTRUCTION_STATISTICS
	this->instruction_histogram[0xCB00 | opcode]++;
#endif
	(this->*function_pointer)();
}
