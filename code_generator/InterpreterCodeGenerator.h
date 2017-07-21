#include "CodeGenerator.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <array>
#include <sstream>

class InterpreterCodeGenerator : public CodeGenerator{
	struct Function{
		int opcode;
		bool double_opcode;
		bool opcode_is_jump;
		std::stringstream contents;
	};
	std::map<std::string, Function> functions;
	struct DefinitionContext{
		std::stringstream *function_contents;
		Function *function;
		unsigned temporary_index;
	};
	std::vector<DefinitionContext> definition_stack;
	std::vector<std::string *> temporary_values;
	std::string class_name;

	std::array<uintptr_t, 3> add(uintptr_t, uintptr_t, unsigned operand_size, unsigned modulo = 8);
protected:
	void begin_opcode_definition(unsigned first) override;
	void end_opcode_definition(unsigned first) override;
	void begin_double_opcode_definition(unsigned first, unsigned second) override;
	void end_double_opcode_definition(unsigned first, unsigned second) override;
	void opcode_cb_branching() override;
public:
	InterpreterCodeGenerator(std::shared_ptr<CpuDefinition> definition, const char *class_name): CodeGenerator(definition), class_name(class_name){}
	~InterpreterCodeGenerator();
	void dump_definitions(std::ostream &stream);
	void dump_declarations(std::ostream &stream);


	// Overrides:
	void noop() override;
	void halt() override;
	uintptr_t load_program_counter8() override;
	uintptr_t load_program_counter16() override;
	uintptr_t get_register_value8(Register8) override;
	uintptr_t get_register_value16(Register16) override;
	uintptr_t load_hl8() override;
	uintptr_t load_mem8(uintptr_t) override;
	uintptr_t load_io_register(uintptr_t) override;
	uintptr_t load_mem16(uintptr_t) override;
	uintptr_t load_sp_offset16(uintptr_t) override;
	void write_register8(Register8, uintptr_t) override;
	void write_register16(Register16, uintptr_t) override;
	void store_hl8(uintptr_t) override;
	void store_mem8(uintptr_t mem, uintptr_t val) override;
	void store_io_register(uintptr_t mem, uintptr_t val) override;
	void store_mem16(uintptr_t mem, uintptr_t val) override;
	void take_time(unsigned) override;
	void zero_flags() override;
	std::array<uintptr_t, 3> add8(uintptr_t a, uintptr_t b) override{
		return this->add(a, b, 8);
	}
	std::array<uintptr_t, 3> add16_using_carry_modulo_16(uintptr_t a, uintptr_t b) override{
		return this->add(a, b, 16, 16);
	}
	std::array<uintptr_t, 3> add8_carry(uintptr_t, uintptr_t) override;
	std::array<uintptr_t, 3> sub8(uintptr_t, uintptr_t) override;
	std::array<uintptr_t, 3> sub8_carry(uintptr_t, uintptr_t) override;
	uintptr_t sub16_no_carry(uintptr_t a, uintptr_t b);
	uintptr_t and8(uintptr_t, uintptr_t) override;
	uintptr_t xor8(uintptr_t, uintptr_t) override;
	uintptr_t or8(uintptr_t, uintptr_t) override;
	std::array<uintptr_t, 3> cmp8(uintptr_t, uintptr_t) override;
	void set_flags(const FlagSettings &) override;
	uintptr_t plus_1(uintptr_t) override;
	uintptr_t minus_1(uintptr_t) override;
	std::array<uintptr_t, 3> add16(uintptr_t a, uintptr_t b) override{
		return this->add(a, b, 16);
	}
	uintptr_t bitwise_not(uintptr_t) override;
	void disable_interrupts() override;
	void enable_interrupts() override;
	void schedule_interrupt_enable() override;
	uintptr_t rotate8(uintptr_t, bool left, bool through_carry) override;
	void stop() override;
	uintptr_t shift8_left(uintptr_t val) override;
	uintptr_t arithmetic_shift_right(uintptr_t val) override;
	uintptr_t bitwise_shift_right(uintptr_t val) override;
	uintptr_t get_bit_value(uintptr_t val, unsigned bit) override;
	uintptr_t set_bit_value(uintptr_t val, unsigned bit, bool on) override;
	std::pair<uintptr_t, uintptr_t> perform_decimal_adjustment(uintptr_t val) override;
	uintptr_t swap_nibbles(uintptr_t val) override;
	uintptr_t get_imm_value(unsigned val) override;
	void require_equals(uintptr_t, uintptr_t) override;
	void do_nothing_if(uintptr_t, unsigned take_time, bool invert = false) override;
	uintptr_t condition_to_value(ConditionalJumpType) override;
	void abort() override;
	uintptr_t sign_extend8(uintptr_t) override;
};
