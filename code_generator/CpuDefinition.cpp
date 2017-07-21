#include "CpuDefinition.h"
#include "CodeGenerator.h"
#include "FlagSetting.h"
#include <memory>
#include <cassert>
#include <cstring>
#include <array>

bool match_opcode(unsigned opcode, const char *pattern){
	auto length = strlen(pattern);
	assert(opcode < 0x100 && length == 8);
	for (auto i = length; i--; opcode >>= 1){
		auto match = pattern[i];
		if (match != '0' && match != '1')
			continue;
		bool match_value = match == '1';
		bool bit = (opcode & 1) == 1;
		if (bit != match_value)
			return false;
	}
	return true;
}

void CpuDefinition::generate(unsigned opcode, CodeGenerator &generator){
	switch (opcode){
		case 0x00:
			// Handle no-op.
			generator.opcode_ends();
			generator.take_time(4);
			return;
		case 0x08:
			{
				// Handle SP -> (imm16)16
				auto imm = generator.load_program_counter16();
				generator.opcode_ends();
				auto val = generator.get_register_value16(Register16::SP);
				generator.store_mem16(imm, val);
				generator.take_time(20);
				return;
			}
		case 0x10:
			{
				auto imm = generator.load_program_counter8();
				generator.opcode_ends();
				generator.require_equals(imm, generator.get_imm_value(0));
				generator.stop();
				generator.take_time(4);
				return;
			}
		case 0x18:
			{
				// Handle imm8 + PC -> PC
				auto imm = generator.load_program_counter8();
				generator.opcode_ends();
				auto val = generator.get_register_value16(Register16::PC);
				auto se = generator.sign_extend8(imm);
				val = generator.add16(val, se)[0];
				generator.write_register16(Register16::PC, val);
				generator.take_time(8);
				return;
			}
		case 0x27:
			{
				// Handle DAA A
				generator.opcode_ends();
				auto val = generator.get_register_value8(Register8::A);
				auto pair = generator.perform_decimal_adjustment(val);
				val = pair.first;
				generator.write_register8(Register8::A, val);
				generator.set_flags({ FlagSetting::IfZero(val), FlagSetting::Keep, FlagSetting::Reset, FlagSetting::IfNonZero(pair.second) });
				generator.take_time(4);
				return;
			}
		case 0x2F:
			// Handle ~A -> A
			{
				generator.opcode_ends();
				auto val = generator.get_register_value8(Register8::A);
				val = generator.bitwise_not(val);
				generator.write_register8(Register8::A, val);
				generator.set_flags({ FlagSetting::Keep, FlagSetting::Set, FlagSetting::Set, FlagSetting::Keep });
				generator.take_time(4);
				return;
			}
		case 0x37:
			// Handle 1 -> CarryFlag
			generator.opcode_ends();
			generator.set_flags({ FlagSetting::Keep, FlagSetting::Reset, FlagSetting::Reset, FlagSetting::Set });
			generator.take_time(4);
			return;
		case 0x3F:
			// Handle !CarryFlag -> CarryFlag
			generator.opcode_ends();
			generator.set_flags({ FlagSetting::Keep, FlagSetting::Reset, FlagSetting::Reset, FlagSetting::Flip });
			generator.take_time(4);
			return;
		case 0x76:
			generator.opcode_ends();
			generator.halt();
			generator.take_time(4);
			return;
		case 0xC3:
			{
				// Handle imm16 -> PC
				auto imm = generator.load_program_counter16();
				generator.opcode_ends();
				generator.write_register16(Register16::PC, imm);
				generator.take_time(12);
				return;
			}
		case 0xCB:
			generator.double_opcode(opcode);
			return;
		case 0xC9:
		case 0xD9:
			{
				// Handle RET, RETI
				generator.opcode_ends();
				auto old_sp = generator.get_register_value16(Register16::SP);
				auto addr = generator.load_mem16(old_sp);
				generator.inc2_SP(old_sp);
				generator.write_register16(Register16::PC, addr);
				if (opcode == 0xD9)
					generator.enable_interrupts();
				generator.take_time(8);
				return;
			}
		case 0xCD:
			{
				// Handle CALL imm16
				auto imm = generator.load_program_counter16();
				generator.opcode_ends();
				generator.push_PC();
				generator.write_register16(Register16::PC, imm);
				generator.take_time(12);
				return;
			}
		case 0xE8:
			{
				// Handle imm16 + SP -> SP
				auto imm = generator.load_program_counter16();
				generator.opcode_ends();
				auto reg = generator.get_register_value16(Register16::SP);
				auto sum = generator.add16(reg, imm);
				generator.write_register16(Register16::SP, sum[0]);
				generator.set_flags({ FlagSetting::Reset, FlagSetting::Reset, FlagSetting::IfNonZero(sum[1]), FlagSetting::IfNonZero(sum[2]) });
				generator.take_time(16);
				return;
			}
		case 0xE9:
			{
				// Handle HL -> SP
				generator.opcode_ends();
				auto reg = generator.get_register_value16(Register16::HL);
				generator.write_register16(Register16::PC, reg);
				generator.take_time(4);
				return;
			}
		case 0xEA:
			{
				// Handle A -> (imm16)8
				auto imm = generator.load_program_counter16();
				generator.opcode_ends();
				auto reg = generator.get_A();
				generator.store_mem8(imm, reg);
				auto mem = generator.load_mem8(imm);
				generator.take_time(16);
				return;
			}
		case 0xF3:
			generator.opcode_ends();
			generator.disable_interrupts();
			generator.take_time(4);
			return;
		case 0xF8:
			{
				// Handle SP + imm8 -> HL
				auto imm = generator.load_program_counter8();
				generator.opcode_ends();
				auto sp = generator.get_register_value16(Register16::SP);
				auto sum = generator.add16(sp, imm);
				auto val = sum[0];
				generator.write_register16(Register16::HL, val);
				generator.set_flags({ FlagSetting::Reset, FlagSetting::Reset, FlagSetting::IfNonZero(sum[1]), FlagSetting::IfNonZero(sum[2]) });
				generator.take_time(12);
				return;
			}
		case 0xF9:
			// Handle HL -> SP
			generator.opcode_ends();
			generator.write_register16(Register16::SP, generator.get_register_value16(Register16::HL));
			generator.take_time(8);
			return;
		case 0xFB:
			// Handle EI (enable interrupts)
			generator.opcode_ends();
			generator.schedule_interrupt_enable();
			generator.take_time(4);
			return;
		case 0xFA:
			{
				// Handle (imm16)8 -> A
				auto imm = generator.load_program_counter16();
				generator.opcode_ends();
				auto mem = generator.load_mem8(imm);
				generator.write_A(mem);
				generator.take_time(16);
				return;
			}
	}

	// 000xx010
	if (match_opcode(opcode, "000xx010")){
		generator.opcode_ends();
		auto operand = (Register16A)((opcode >> 4) & 1);
		auto operation = (opcode >> 3) & 1;
		if (operation){
			// Handle (BC)8 -> A, (DE)8 -> A.
			auto reg = generator.get_register_value16(operand);
			auto mem = generator.load_mem8(reg);
			generator.write_A(mem);
		}else{
			// Handle A -> (BC)8, A -> (DE)8.
			auto dst = generator.get_register_value16(operand);
			auto src = generator.get_A();
			generator.store_mem8(dst, src);
		}
		generator.take_time(8);
		return;
	}

	// 000xx111
	if (match_opcode(opcode, "000xx111")){
		// Handle rotations
		generator.opcode_ends();
		bool left = !((opcode >> 3) & 1);
		bool using_carry = !!((opcode >> 4) & 1);
		auto val = generator.get_register_value8(Register8::A);
		auto old_a = val;
		val = generator.rotate8(val, left, using_carry);
		int bit = left * 7;
		auto carry = FlagSetting::IfNonZero(generator.get_bit_value(old_a, bit));
		FlagSettings fs = { FlagSetting::Reset, FlagSetting::Reset, FlagSetting::Reset, carry };
		generator.write_register8(Register8::A, val);
		generator.set_flags(fs);
		generator.take_time(4);
		return;
	}

	// 00xxx011
	if (match_opcode(opcode, "00xxx011")){
		// Handle INC reg16, DEC reg16
		generator.opcode_ends();
		auto reg = to_Register16((Register16A)((opcode >> 4) & 3));
		bool decrement = (opcode >> 3) & 1;
		auto val = generator.get_register_value16(reg);
		if (decrement)
			val = generator.minus_1(val);
		else
			val = generator.plus_1(val);
		generator.write_register16(reg, val);
		generator.take_time(8);
		return;
	}

	// 00xx0001
	if (match_opcode(opcode, "00xx0001")){
		// Handle imm16 -> reg16
		auto reg = (Register16A)((opcode >> 4) & 3);
		auto imm = generator.load_program_counter16();
		generator.opcode_ends();
		generator.write_register16(reg, imm);
		generator.take_time(12);
		return;
	}

	// 00xxx110
	if (match_opcode(opcode, "00xxx110")){
		// Handle imm8 -> reg8
		auto dst = (Register8)((opcode >> 3) & 7);
		auto imm = generator.load_program_counter8();
		generator.opcode_ends();
		if (dst == Register8::None){
			generator.store_hl8(imm);
			generator.take_time(12);
			return;
		}
		generator.write_register8(dst, imm);
		generator.take_time(8);
		return;
	}

	// 00xx1001
	if (match_opcode(opcode, "00xx1001")){
		// Handle reg16 + HL -> HL
		generator.opcode_ends();
		auto reg = to_Register16((Register16A)((opcode >> 4) & 3));
		auto hl = generator.get_register_value16(Register16::HL);
		auto val = generator.get_register_value16(reg);
		auto array = generator.add16_using_carry_modulo_16(val, hl);
		val = array[0];
		generator.write_register16(Register16::HL, val);
		generator.set_flags({ FlagSetting::IfZero(val), FlagSetting::Reset, FlagSetting::IfNonZero(array[1]), FlagSetting::IfNonZero(array[2]) });
		generator.take_time(8);
		return;
	}

	// 00xxx10x
	if (match_opcode(opcode, "00xxx10x")){
		//Handle INC reg8, DEC reg8
		generator.opcode_ends();
		bool decrement = (opcode & 1) == 1;
		auto operand = (Register8)((opcode >> 3) & 7);
		unsigned time;
		uintptr_t val = 0;
		uintptr_t one = generator.get_imm_value(1);
		uintptr_t addr = 0;

		if (operand == Register8::None){
			addr = generator.get_register_value16(Register16::HL);
			val = generator.load_mem8(addr);
			time = 12;
		}else{
			val = generator.get_register_value8(operand);
			time = 4;
		}

		std::array<uintptr_t, 3> array;
		if (decrement)
			array = generator.sub8(val, one);
		else
			array = generator.add8(val, one);
		val = array[0];

		if (operand == Register8::None)
			generator.store_mem8(addr, val);
		else
			generator.write_register8(operand, val);

		auto half_carry = FlagSetting::Keep;
		auto sub = FlagSetting::Keep;

		if (decrement){
			sub = FlagSetting::Set;
			half_carry = FlagSetting::IfNonZero(array[1]);
		}else{
			sub = FlagSetting::Reset;
			half_carry = FlagSetting::IfNonZero(array[1]);
		}

		generator.set_flags({ FlagSetting::IfZero(val), sub, half_carry, FlagSetting::Keep });
		generator.take_time(time);
		return;
	}

	// 001xx010
	if (match_opcode(opcode, "001xx010")){
		generator.opcode_ends();
		bool decrement = (opcode >> 4) & 1;
		bool load = (opcode >> 3) & 1;

		auto hl = generator.get_register_value16(Register16::HL);

		if (load)
			generator.write_A(generator.load_mem8(hl));
		else
			generator.store_mem8(hl, generator.get_A());

		if (decrement)
			hl = generator.minus_1(hl);
		else
			hl = generator.plus_1(hl);

		generator.write_register16(Register16::HL, hl);

		generator.take_time(8);
		return;
	}

	// 01xxxxxx
	if (match_opcode(opcode, "01xxxxxx")){
		assert(opcode != 0x76);
		// Handle reg8 -> reg8, reg8 -> (HL)8, and (HL)8 -> reg8.
		generator.opcode_ends();
		auto dst = (Register8)((opcode >> 3) & 7);
		auto src = (Register8)(opcode & 7);
		if (dst == src){
			assert(dst != Register8::None && src != Register8::None);
			generator.noop();
			generator.take_time(4);
			return;
		}
		if (src == Register8::None){
			assert(dst != Register8::None);
			auto mem = generator.load_hl8();
			generator.write_register8(dst, mem);
			generator.take_time(8);
			return;
		}else if (dst == Register8::None){
			assert(src != Register8::None);
			auto reg = generator.get_register_value8(src);
			generator.store_hl8(reg);
			generator.take_time(8);
			return;
		}
		auto reg = generator.get_register_value8(src);
		generator.write_register8(dst, reg);
		generator.take_time(4);
		return;
	}

	// 10xxxxxx
	// 11xxx110
	if (match_opcode(opcode, "10xxxxxx") || match_opcode(opcode, "11xxx110")){
		auto operation = (opcode >> 3) & 7;
		auto operand = (Register8)(opcode & 7);
		auto immediate = (opcode & 0x40) == 0x40;
		uintptr_t val;
		if (!immediate){
			generator.opcode_ends();
			if (operand == Register8::None)
				val = generator.load_hl8();
			else
				val = generator.get_register_value8(operand);
		}else{
			val = generator.load_program_counter8();
			generator.opcode_ends();
		}
		uintptr_t regA = generator.get_register_value8(Register8::A);
		std::array<uintptr_t, 3> array = { 0, 0, 0 };
		bool subtraction = false;
		switch (operation){
			case 0:
				array = generator.add8(regA, val);
				val = array[0];
				break;
			case 1:
				array = generator.add8_carry(regA, val);
				val = array[0];
				break;
			case 2:
				array = generator.sub8(regA, val);
				val = array[0];
				subtraction = true;
				break;
			case 3:
				array = generator.sub8_carry(regA, val);
				val = array[0];
				subtraction = true;
				break;
			case 4:
				val = generator.and8(regA, val);
				break;
			case 5:
				val = generator.xor8(regA, val);
				break;
			case 6:
				val = generator.or8(regA, val);
				break;
			case 7:
				array = generator.cmp8(regA, val);
				val = array[0];
				subtraction = true;
				break;
		}
		if (operation != 7)
			generator.write_register8(Register8::A, val);
		auto if_zero = FlagSetting::IfZero(val);
		FlagSetting half_carry = FlagSetting::Keep,
			carry = FlagSetting::Reset;
		if (array[0]){
			//if (!subtraction){
				half_carry = FlagSetting::IfNonZero(array[1]);
				carry = FlagSetting::IfNonZero(array[2]);
			//}else{
			//	half_carry = FlagSetting::IfZero(array[1]);
			//	carry = FlagSetting::IfZero(array[2]);
			//}
		}
		FlagSettings fs = { FlagSetting::Keep, FlagSetting::Keep, FlagSetting::Keep, FlagSetting::Keep };
		switch (operation){
			case 0:
			case 1:
				fs = { if_zero, FlagSetting::Reset, half_carry, carry };
				break;
			case 2:
			case 3:
				fs = { if_zero, FlagSetting::Set, half_carry, carry };
				break;
			case 4:
				fs = { if_zero, FlagSetting::Reset, FlagSetting::Set, carry };
				break;
			case 5:
			case 6:
				fs = { if_zero, FlagSetting::Reset, FlagSetting::Reset, carry };
				break;
			case 7:
				fs = { if_zero, FlagSetting::Set, half_carry, carry };
				break;
		}
		generator.set_flags(fs);
		generator.take_time(operand == Register8::None ? 8 : 4);
		return;
	}

	// 11xx0x01
	if (match_opcode(opcode, "11xx0x01")){
		// Handle PUSH, POP
		generator.opcode_ends();
		auto reg = to_Register16((Register16B)((opcode >> 4) & 3));
		auto push = (opcode & 4) == 4;
		if (push){
			auto addr = generator.dec2_SP();
			auto val = generator.get_register_value16(reg);
			generator.store_mem16(addr, val);
		}else{
			auto old_sp = generator.get_register_value16(Register16::SP);
			auto val = generator.load_mem16(old_sp);
			generator.write_register16(reg, val);
			generator.inc2_SP(old_sp);
		}
		generator.take_time(push ? 16 : 12);
		return;
	}

	if (match_opcode(opcode, "11xxx111")){
		// Handle RST
		generator.opcode_ends();
		auto addr = opcode & 0x38;
		generator.push_PC();
		auto imm = generator.get_imm_value(addr);
		generator.write_register16(Register16::PC, imm);
		generator.take_time(16);
		return;
	}

	// 110xx000
	if (match_opcode(opcode, "110xx000")){
		// Handle conditional RET
		generator.opcode_ends();
		auto type = (ConditionalJumpType)((opcode >> 3) & 3);
		generator.do_nothing_if(generator.condition_to_value(type), 8, true);
		auto old_sp = generator.get_register_value16(Register16::SP);
		auto addr = generator.load_mem16(old_sp);
		generator.inc2_SP(old_sp);
		generator.write_register16(Register16::PC, addr);
		generator.take_time(20);
		return;
	}

	// 110xx010
	// 001xx000
	if (match_opcode(opcode, "110xx010") || match_opcode(opcode, "001xx000")){
		// Handle conditional relative and absolute jumps.
		bool absolute = (opcode & 0x80) == 0x80;
		auto type = (ConditionalJumpType)((opcode >> 3) & 3);
		unsigned time;
		if (absolute){
			time = 12;
			auto imm = generator.load_program_counter16();
			generator.opcode_ends();
			generator.do_nothing_if(generator.condition_to_value(type), time, true);
			time += 4;
			generator.write_register16(Register16::PC, imm);
		}else{
			time = 8;
			auto imm = generator.load_program_counter8();
			generator.opcode_ends();
			generator.do_nothing_if(generator.condition_to_value(type), time, true);
			time += 4;
			auto value = generator.add16(generator.get_register_value16(Register16::PC), generator.sign_extend8(imm))[0];
			generator.write_register16(Register16::PC, value);
		}
		generator.take_time(time);
		return;
	}

	// 110xx100
	if (match_opcode(opcode, "110xx100")){
		// Handle conditional calls.
		auto type = (ConditionalJumpType)((opcode >> 3) & 3);
		auto imm = generator.load_program_counter16();
		generator.opcode_ends();
		generator.do_nothing_if(generator.condition_to_value(type), 12, true);
		generator.push_PC();
		generator.write_register16(Register16::PC, imm);
		generator.take_time(24);
		return;
	}

	// 111x00x0
	if (match_opcode(opcode, "111x00x0")){
		// Handle (0xFF00 + C)8 -> A, (0xFF00 + imm8)8 -> A
		// Handle A -> (0xFF00 + C)8, A -> (0xFF00 + imm8)8

		bool using_C = (opcode & 2) == 2;
		bool load = (opcode & 0x10) == 0x10;
		uintptr_t offset;
		if (using_C){
			generator.opcode_ends();
			offset = generator.get_register_value8(Register8::C);
		}else{
			offset = generator.load_program_counter8();
			generator.opcode_ends();
		}

		auto base = generator.get_imm_value(0xFF00);
		auto addr = generator.add16(base, offset)[0];

		if (load)
			generator.write_A(generator.load_mem8(addr));
		else
			generator.store_mem8(addr, generator.get_A());

		generator.take_time(using_C ? 8 : 12);
		return;
	}

	generator.opcode_ends();
	generator.abort();
}

void CpuDefinition::generate(unsigned first_opcode, unsigned opcode, CodeGenerator &generator){
	if (first_opcode != 0xCB)
		abort();

	generator.opcode_ends();

	// 00xxxxxx
	if (match_opcode(opcode, "00xxxxxx")){
		auto operation = (BitwiseOps)((opcode >> 3) & 7);
		auto operand = (Register8)(opcode & 7);
		unsigned time = 8;
		uintptr_t val;
		if (operand == Register8::None){
			val = generator.load_hl8();
			time = 16;
		}else
			val = generator.get_register_value8(operand);
		auto carry = FlagSetting::Reset;
		auto old_bit_7 = FlagSetting::IfNonZero(generator.get_bit_value(val, 7));
		auto old_bit_0 = FlagSetting::IfNonZero(generator.get_bit_value(val, 0));
		switch (operation){
			case BitwiseOps::RotateLeftUsingCarry:
				val = generator.rotate8(val, true, true);
				carry = old_bit_7;
				break;
			case BitwiseOps::RotateRightUsingCarry:
				val = generator.rotate8(val, false, true);
				carry = old_bit_0;
				break;
			case BitwiseOps::RotateLeftNotUsingCarry:
				val = generator.rotate8(val, true, false);
				carry = old_bit_7;
				break;
			case BitwiseOps::RotateRightNotUsingCarry:
				val = generator.rotate8(val, false, false);
				carry = old_bit_0;
				break;
			case BitwiseOps::ShiftLeft:
				val = generator.shift8_left(val);
				carry = old_bit_7;
				break;
			case BitwiseOps::ArithmeticShiftRight:
				val = generator.arithmetic_shift_right(val);
				carry = old_bit_0;
				break;
			case BitwiseOps::Swap:
				val = generator.swap_nibbles(val);
				break;
			case BitwiseOps::BitwiseShiftRight:
				val = generator.bitwise_shift_right(val);
				carry = old_bit_0;
				break;
		}
		if (operand == Register8::None)
			generator.store_hl8(val);
		else
			generator.write_register8(operand, val);
		generator.set_flags({ FlagSetting::IfZero(val), FlagSetting::Reset, FlagSetting::Reset, carry });
		generator.take_time(time);
		return;
	}

	auto operation = (BitfieldOps)((opcode >> 6) & 3);
	auto register_operand = (Register8)(opcode & 7);
	auto bit_operand = (opcode >> 3) & 7;
	unsigned time = 8;
	uintptr_t val;
	if (register_operand == Register8::None){
		val = generator.load_hl8();
		time = 16;
	}else
		val = generator.get_register_value8(register_operand);
	bool store_back = false;
	switch (operation){
		case BitfieldOps::BitCheck:
			val = generator.get_bit_value(val, bit_operand);
			generator.set_flags({ FlagSetting::IfZero(val), FlagSetting::Reset, FlagSetting::Set, FlagSetting::Keep });
			break;
		case BitfieldOps::BitReset:
			val = generator.set_bit_value(val, bit_operand, false);
			store_back = true;
			break;
		case BitfieldOps::BitSet:
			val = generator.set_bit_value(val, bit_operand, true);
			store_back = true;
			break;
		default:
			abort();
			break;
	}
	if (store_back){
		if (register_operand == Register8::None)
			generator.store_hl8(val);
		else
			generator.write_register8(register_operand, val);
	}
	generator.take_time(time);
}
