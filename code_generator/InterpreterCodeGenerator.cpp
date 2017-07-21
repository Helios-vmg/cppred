#include "InterpreterCodeGenerator.h"
#include "FlagSetting.h"
#include <iomanip>
#include <type_traits>
#include <algorithm>

#define TEMPTYPE "main_integer_t"
#define TEMPDECL "\t" TEMPTYPE " "
#define CONSTTEMPDECL "\tconst " TEMPTYPE " "

//#define GENERATE_CHECKS

static std::string get_temp_name(unsigned i){
	std::stringstream stream;
	stream << "temp" << i;
	return stream.str();
}

template <typename T>
static T *copy(const T &x){
	return new T(x);
}

template <typename T, size_t N>
static constexpr size_t array_length(const T (&)[N]){
	return N;
}

static const char *to_string_r(Register8 reg){
	switch (reg){
		case Register8::B:
			return "this->registers.get_b()";
		case Register8::C:
			return "this->registers.get_c()";
		case Register8::D:
			return "this->registers.get_d()";
		case Register8::E:
			return "this->registers.get_e()";
		case Register8::H:
			return "this->registers.get_h()";
		case Register8::L:
			return "this->registers.get_l()";
		case Register8::None:
			return nullptr;
		case Register8::A:
			return "this->registers.get_a()";
		default:
			return nullptr;
	}
}

static const char *to_string_w(Register8 reg){
	switch (reg){
		case Register8::B:
			return "this->registers.set_b()";
		case Register8::C:
			return "this->registers.set_c()";
		case Register8::D:
			return "this->registers.set_d()";
		case Register8::E:
			return "this->registers.set_e()";
		case Register8::H:
			return "this->registers.set_h()";
		case Register8::L:
			return "this->registers.set_l()";
		case Register8::None:
			return nullptr;
		case Register8::A:
			return "this->registers.set_a()";
		default:
			return nullptr;
	}
}

static const char *to_string_r(Register16 reg){
	switch (reg){
		case Register16::AF:
			return "this->registers.get_af()";
		case Register16::BC:
			return "this->registers.get_bc()";
		case Register16::DE:
			return "this->registers.get_de()";
		case Register16::HL:
			return "this->registers.get_hl()";
		case Register16::SP:
			return "this->registers.get_sp()";
		case Register16::PC:
			return "this->registers.get_pc()";
		default:
			return nullptr;
	}
}

static const char *to_string_w(Register16 reg){
	switch (reg){
		case Register16::AF:
			return "this->registers.set_af()";
		case Register16::BC:
			return "this->registers.set_bc()";
		case Register16::DE:
			return "this->registers.set_de()";
		case Register16::HL:
			return "this->registers.set_hl()";
		case Register16::SP:
			return "this->registers.set_sp()";
		case Register16::PC:
			return "this->registers.set_pc()";
		default:
			return nullptr;
	}
}

template <typename T>
static typename std::enable_if<std::is_integral<T>::value, std::string>::type to_string(T n){
	std::stringstream s;
	s << n;
	return s.str();
}

static const std::string &temp_to_string(uintptr_t p){
	return *(std::string *)p;
}

InterpreterCodeGenerator::~InterpreterCodeGenerator(){
	for (auto p : this->temporary_values)
		delete p;
}

#define OPCODE_TABLE_INIT_FUNCTION "initialize_opcode_tables"
#define MAIN_OPCODE_TABLE "opcode_table"
#define SECOND_OPCODE_TABLE "opcode_table_cb"
#define MAIN_JUMPS_TABLE "opcode_is_jump_table"
#define SECOND_JUMPS_TABLE "opcode_is_jump_table_cb"

void InterpreterCodeGenerator::dump_declarations(std::ostream &stream){
	stream
		<< "typedef void (" << this->class_name << "::*opcode_function_pointer)();\n"
		<< "opcode_function_pointer " MAIN_OPCODE_TABLE "[256];\n"
		<< "opcode_function_pointer " SECOND_OPCODE_TABLE "[256];\n"
		<< "static const bool " MAIN_JUMPS_TABLE "[256];\n"
		<< "static const bool " SECOND_JUMPS_TABLE "[256];\n"
		<< "void " OPCODE_TABLE_INIT_FUNCTION "();\n";
	for (auto &kv : this->functions)
		stream << "void " << kv.first << "();\n";
	stream << "\n";
}

void InterpreterCodeGenerator::dump_definitions(std::ostream &stream){
	stream << "#include <GameboyCpu.h>\n\n";

	// Functions.

	stream << "void " << this->class_name << "::" OPCODE_TABLE_INIT_FUNCTION "(){\n";
	for (auto &kv : this->functions){
		if (kv.second.double_opcode)
			continue;
		stream << "\tthis->" MAIN_OPCODE_TABLE "[" << kv.second.opcode << "] = &" << this->class_name << "::" << kv.first << ";\n";
	}
	for (auto &kv : this->functions){
		if (!kv.second.double_opcode)
			continue;
		stream << "\tthis->" SECOND_OPCODE_TABLE "[" << kv.second.opcode << "] = &" << this->class_name << "::" << kv.first << ";\n";
	}
	stream << "}\n\n";

	for (auto &kv : this->functions){
		if (kv.first == "opcode_cb")
			continue;
		stream
			<< "void " << this->class_name << "::" << kv.first << "(){\n"
			<< kv.second.contents.str()
			<< "}\n\n";
	}

	// Tables.

	stream << "const bool " << this->class_name << "::" MAIN_JUMPS_TABLE "[256] = { ";
	for (auto &kv : this->functions){
		if (kv.second.double_opcode)
			continue;
		stream << (kv.second.opcode_is_jump ? "true, " : "false, ");
	}
	stream << "};\n\n";

	stream << "const bool " << this->class_name << "::" SECOND_JUMPS_TABLE "[256] = { ";
	for (auto &kv : this->functions){
		if (!kv.second.double_opcode)
			continue;
		stream << (kv.second.opcode_is_jump ? "true, " : "false, ");
	}
	stream << "};\n\n";
}

void InterpreterCodeGenerator::begin_opcode_definition(unsigned first){
	std::stringstream stream;
	stream << "opcode_" << std::hex << std::setw(2) << std::setfill('0') << first;
	auto name = stream.str();
	auto &value = this->functions[name];
	value.opcode = first;
	value.double_opcode = false;
	value.opcode_is_jump = false;
	auto contents = &value.contents;
	this->definition_stack.push_back({ contents, &value, 0 });
}

void InterpreterCodeGenerator::end_opcode_definition(unsigned first){
	this->definition_stack.pop_back();
}

void InterpreterCodeGenerator::begin_double_opcode_definition(unsigned first, unsigned second){
	std::stringstream stream;
	stream
		<< "opcode_"
		<< std::hex << std::setw(2) << std::setfill('0') << first
		<< std::hex << std::setw(2) << std::setfill('0') << second;
	auto name = stream.str();
	auto &value = this->functions[name];
	value.opcode = second;
	value.double_opcode = true;
	value.opcode_is_jump = false;
	auto contents = &value.contents;
	this->definition_stack.push_back({ contents, &value, 0 });
}

void InterpreterCodeGenerator::end_double_opcode_definition(unsigned first, unsigned second){
	this->definition_stack.pop_back();
}

void InterpreterCodeGenerator::opcode_cb_branching(){
	auto &s = *this->definition_stack.back().function_contents;
	auto &opcode = *(std::string *)this->load_program_counter8();
	s
		<< "\tauto function_pointer = this->" << SECOND_OPCODE_TABLE << "[" << opcode << "];\n"
		<< "\t(this->*function_pointer)();\n";
}

void InterpreterCodeGenerator::noop(){
}

void InterpreterCodeGenerator::halt(){
	auto &s = *this->definition_stack.back().function_contents;
	s << "\tthis->halt();\n";
}

uintptr_t InterpreterCodeGenerator::load_program_counter8(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = this->load_pc_and_increment();\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_program_counter16(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto temp_name = get_temp_name(n++);
	auto result_name = get_temp_name(n++);
	s
		<< TEMPDECL << temp_name << " = " << to_string_r(Register16::PC) << ";\n"
		<< "\t" << to_string_w(Register16::PC) << " = (std::uint16_t)(" << temp_name << " + 2);\n"
		<< TEMPDECL << result_name << " = this->memory_controller.load16(" << temp_name << ");\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_register_value8(Register8 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << to_string_r(reg) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_register_value16(Register16 reg){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << to_string_r(reg) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_hl8(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = this->memory_controller.load8(" << to_string_r(Register16::HL) << ");\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_mem8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = this->memory_controller.load8(" << temp_to_string(val) << ");\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_io_register(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = this->memory_controller.load8_io(" << temp_to_string(val) << ");\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_mem16(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = this->memory_controller.load16(" << temp_to_string(val) << ");\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::load_sp_offset16(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = this->memory_controller.load16(" << get_register_value16(Register16::SP) << " + " << temp_to_string(val) << ");\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::write_register8(Register8 reg, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\t" << to_string_w(reg) << " = (byte_t)" << temp_to_string(val) << ";\n";
}

void InterpreterCodeGenerator::write_register16(Register16 reg, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\t" << to_string_w(reg) << " = (std::uint16_t)" << temp_to_string(val) << ";\n";
	if (reg == Register16::PC)
		back.function->opcode_is_jump = true;
}

void InterpreterCodeGenerator::store_hl8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(" << to_string_r(Register16::HL) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::store_mem8(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8(" << temp_to_string(mem) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::store_io_register(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store8_io(" << temp_to_string(mem) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::store_mem16(uintptr_t mem, uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->memory_controller.store16(" << temp_to_string(mem) << ", " << temp_to_string(val) << ");\n";
}

void InterpreterCodeGenerator::take_time(unsigned cycles){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->take_time(" << cycles << ");\n";
}

void InterpreterCodeGenerator::zero_flags(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->set(Register8::Flags, 0);\n";
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::add(uintptr_t valA, uintptr_t valB, unsigned operand_size, unsigned modulo){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto carrybits_name = get_temp_name(n++);
	auto half_carry_name = get_temp_name(n++);
	auto full_carry_name = get_temp_name(n++);
	auto &valA_name = temp_to_string(valA);
	auto &valB_name = temp_to_string(valB);
	unsigned half_carry_mask, full_carry_mask;
	if (modulo == 8){
		half_carry_mask = 0x10;
		full_carry_mask = 0x100;
	}else if (modulo == 16){
		half_carry_mask = 0x1000;
		full_carry_mask = 0x10000;
	}else
		abort();
	s
		<< TEMPDECL << result_name << " = " << valA_name << " + " << valB_name << ";\n"
		<< TEMPDECL << carrybits_name << " = " << valA_name << " ^ " << valB_name << " ^ " << result_name << ";\n"
		<< TEMPDECL << half_carry_name << " = " << carrybits_name << " & " << half_carry_mask << ";\n"
		<< TEMPDECL << full_carry_name << " = " << carrybits_name << " & " << full_carry_mask << ";\n";

	if (operand_size == 8)
		s << "\t" << result_name << " = " << result_name << " & 0xFF;\n";
	else if (operand_size == 16)
		s << "\t" << result_name << " = " << result_name << " & 0xFFFF;\n";
	else
		abort();

	std::string *retp[] = { copy(result_name), copy(half_carry_name), copy(full_carry_name) };
	std::array<uintptr_t, 3> ret;
	for (auto i = 3; i--;){
		this->temporary_values.push_back(retp[i]);
		ret[i] = (uintptr_t)retp[i];
	}
	return ret;
}

uintptr_t InterpreterCodeGenerator::sub16_no_carry(uintptr_t a, uintptr_t b){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto &valA_name = temp_to_string(a);
	auto &valB_name = temp_to_string(b);
	s << TEMPDECL << result_name << " = " << valA_name << " - " << valB_name << ";\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::add8_carry(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto carry_name = get_temp_name(n++);
	auto &valA_name = temp_to_string(valA);
	auto &valB_name = temp_to_string(valB);
	auto result_name = get_temp_name(n++);
	auto half_carry_name = get_temp_name(n++);
	auto full_carry_name = get_temp_name(n++);

	s
		<< TEMPDECL << carry_name << " = this->registers.get(Flags::Carry);\n"
		<< TEMPDECL << result_name << " = " << valA_name << " + " << valB_name << " + " << carry_name << ";\n"
		<< TEMPDECL << half_carry_name << " = (" << valA_name << " & 0x0F) + (" << valB_name << " & 0x0F) + " << carry_name << " >= 0x0F;\n"
		<< TEMPDECL << full_carry_name << " = " << result_name << " & " << 0x100 << ";\n"
		<< "\t" << result_name << " &= 0xFF;\n";

	std::string *retp[] = { copy(result_name), copy(half_carry_name), copy(full_carry_name) };
	std::array<uintptr_t, 3> ret;
	for (auto i = 3; i--;){
		this->temporary_values.push_back(retp[i]);
		ret[i] = (uintptr_t)retp[i];
	}
	return ret;
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::sub8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto hc_temp_name = get_temp_name(n++);
	auto half_carry_name = get_temp_name(n++);
	auto full_carry_name = get_temp_name(n++);
	auto &valA_name = temp_to_string(valA);
	auto &valB_name = temp_to_string(valB);
	s
		<< "\tint " << result_name << " = (int)(" << valA_name << ") - (int)(" << valB_name << ");\n"
		<< "\tint " << hc_temp_name << " = (int)((" << valA_name << ") & 0x0F) - (int)((" << valB_name << ") & 0x0F);\n"
		<< TEMPDECL << half_carry_name << " = " << hc_temp_name << " < 0;\n"
		<< TEMPDECL << full_carry_name << " = " << result_name << " < 0;\n"
		<< "\t" << result_name << " = (int)int_to_uint8(" << result_name << ");\n";

	std::string *retp[] = { copy(result_name), copy(half_carry_name), copy(full_carry_name) };
	std::array<uintptr_t, 3> ret;
	for (auto i = 3; i--;){
		this->temporary_values.push_back(retp[i]);
		ret[i] = (uintptr_t)retp[i];
	}

#ifdef GENERATE_CHECKS
	auto check_carry_name = get_temp_name(n++);
	auto check_half_carry_name = get_temp_name(n++);
	s << "\tint " << check_carry_name << " = (int)" << temp_to_string(valA) << " - " << " (int)" << temp_to_string(valB) << ";\n";
	s << "\tint " << check_half_carry_name << " = (int)((" << temp_to_string(valA) << ") & 0x0F) - " << " (int)((" << temp_to_string(valB) << ") & 0x0F);\n";
	s << "\tif (" << check_carry_name << " < 0 && !!!(" << temp_to_string(ret[1]) << ") || " << check_half_carry_name << " < 0 && !!!(" << temp_to_string(ret[2]) << ")){\n";
	s << "\t\t__debugbreak();\n";
	s << "\t}\n";
	s << "\tif (" << check_carry_name << " >= 0 && !!(" << temp_to_string(ret[1]) << ") || " << check_half_carry_name << " >= 0 && !!(" << temp_to_string(ret[2]) << ")){\n";
	s << "\t\t__debugbreak();\n";
	s << "\t}\n";
#endif

	return ret;
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::sub8_carry(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto hc_temp_name = get_temp_name(n++);
	auto intermediate_name = get_temp_name(n++);
	auto half_carry_name = get_temp_name(n++);
	auto full_carry_name = get_temp_name(n++);
	auto temp_carry = get_temp_name(n++);
	auto &valA_name = temp_to_string(valA);
	auto &valB_name = temp_to_string(valB);
	s
		<< "\tint " << temp_carry << " = this->registers.get(Flags::Carry);\n"
		<< "\tint " << result_name << " = (int)(" << valA_name << ") - (int)(" << valB_name << ") - " << temp_carry << ";\n"
		<< "\tint " << hc_temp_name << " = (int)((" << valA_name << ") & 0x0F) - (int)(" << valB_name << " & 0x0F) - " << temp_carry << ";\n"
		<< TEMPDECL << half_carry_name << " = " << hc_temp_name << " < 0;\n"
		<< TEMPDECL << full_carry_name << " = " << result_name << " < 0;\n"
		<< "\t" << result_name << " = (int)int_to_uint8(" << result_name << ");\n";

	std::string *retp[] = { copy(result_name), copy(half_carry_name), copy(full_carry_name) };
	std::array<uintptr_t, 3> ret;
	for (auto i = 3; i--;){
		this->temporary_values.push_back(retp[i]);
		ret[i] = (uintptr_t)retp[i];
	}


#ifdef GENERATE_CHECKS
	auto check_carry_name = get_temp_name(n++);
	auto check_half_carry_name = get_temp_name(n++);
	s << "\tint " << check_carry_name << " = (int)" << temp_to_string(valA) << " - " << " (int)" << temp_to_string(valB) << ";\n";
	s << "\tint " << check_half_carry_name << " = (int)((" << temp_to_string(valA) << ") & 0x0F) - " << " (int)(((" << temp_to_string(valB) << ") + " << temp_carry << ") & 0x0F);\n";
	s << "\tif (" << check_carry_name << " < 0 && !!!(" << temp_to_string(ret[1]) << ") || " << check_half_carry_name << " < 0 && !!!(" << temp_to_string(ret[2]) << ")){\n";
	s << "\t\t__debugbreak();\n";
	s << "\t}\n";
	s << "\tif (" << check_carry_name << " > 0 && !!(" << temp_to_string(ret[1]) << ") || " << check_half_carry_name << " >= 0 && !!(" << temp_to_string(ret[2]) << ")){\n";
	s << "\t\t__debugbreak();\n";
	s << "\t}\n";
#endif

	return ret;
}

uintptr_t InterpreterCodeGenerator::and8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(valA) << " & " << temp_to_string(valB) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::xor8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(valA) << " ^ " << temp_to_string(valB) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::or8(uintptr_t valA, uintptr_t valB){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(valA) << " | " << temp_to_string(valB) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

std::array<uintptr_t, 3> InterpreterCodeGenerator::cmp8(uintptr_t valA, uintptr_t valB){
	return this->sub8(valA, valB);
}

static std::pair<std::string, std::string> to_string(const FlagSetting &setting){
	std::string temp;
	switch (setting.op){
		case FlagSetting::Operation::Reset:
			return { "(" TEMPTYPE ")0", "(" TEMPTYPE ")0" };
		case FlagSetting::Operation::Set:
			return { "(" TEMPTYPE ")0", "(" TEMPTYPE ")1" };
		case FlagSetting::Operation::Keep:
			return { "(" TEMPTYPE ")1", "(" TEMPTYPE ")0" };
		case FlagSetting::Operation::Flip:
			return { "(" TEMPTYPE ")1", "(" TEMPTYPE ")1" };
		case FlagSetting::Operation::IfNonZero:
			temp = "!";
		case FlagSetting::Operation::IfZero:
			temp += "!";
			temp += temp_to_string(setting.src_value);
			break;
		default:
			abort();
	}
	return{ "0", "(" TEMPTYPE ")" + temp };
}

void InterpreterCodeGenerator::set_flags(const FlagSettings &fs){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	std::pair<FlagSetting, const char *> settings[] = {
		{fs.zero, "(" TEMPTYPE ")7"},
		{fs.subtract, "(" TEMPTYPE ")6"},
		{fs.half_carry, "(" TEMPTYPE ")5"},
		{fs.carry, "(" TEMPTYPE ")4"},
	};
	std::sort(settings, settings + array_length(settings), [](auto &a, auto &b){ return (int)a.first.op < (int)b.first.op; });
	int i = 0;
	std::string A, B;
	bool all_zero_a = true;
	bool all_zero_b = true;
	bool first_a = true;
	bool first_b = true;
	for (auto &setting : settings){
		auto p = to_string(setting.first);

		bool zero_a = setting.first.op != FlagSetting::Operation::Keep && setting.first.op != FlagSetting::Operation::Flip;
		if (!zero_a){
			if (!first_a)
				A += " | ";
			A += "(";
			A += p.first;
			if (setting.second){
				A += " << ";
				A += setting.second;
			}
			A += ")";
			first_a = false;
			all_zero_a = false;
		}

		bool zero_b = setting.first.op == FlagSetting::Operation::Reset || setting.first.op == FlagSetting::Operation::Keep;
		if (!zero_b){
			if (!first_b)
				B += " | ";
			B += "(";
			B += p.second;
			if (setting.second){
				B += " << ";
				B += setting.second;
			}
			B += ")";
			first_b = false;
			all_zero_b = false;
		}
	}
	if (all_zero_a)
		A = "0";
	if (all_zero_b)
		B = "0";
	s << "\tthis->registers.set_flags(" << A << ", " << B << ");\n";
}

uintptr_t InterpreterCodeGenerator::plus_1(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(val) << " + 1;\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::minus_1(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = " << temp_to_string(val) << " - 1;\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::bitwise_not(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	s << TEMPDECL << result_name << " = ~" << temp_to_string(val) << ";\n";
	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::disable_interrupts(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->interrupt_toggle(false);\n";
}

void InterpreterCodeGenerator::enable_interrupts(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->interrupt_toggle(true);\n";
}

void InterpreterCodeGenerator::schedule_interrupt_enable(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->schedule_interrupt_enable();\n";
}

uintptr_t InterpreterCodeGenerator::rotate8(uintptr_t val, bool left, bool using_carry){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto carry_name = get_temp_name(n++);
	if (left){
		if (using_carry){
			s
				<< TEMPDECL << carry_name << " = this->registers.get(Flags::Carry) ? 1U : 0U;\n"
				<< TEMPDECL << result_name << " = (" << temp_to_string(val) << " << 1) | " << carry_name << ";\n";
		}else{
			auto bit_name = get_temp_name(n++);
			s
				<< TEMPDECL << bit_name << " = " << temp_to_string(val) << " & 0xFF;\n"
				<< TEMPDECL << result_name << " = (" << bit_name << " << 1) | (" << bit_name << " >> 7);\n";

		}
	}else{
		if (using_carry){
			s
				<< TEMPDECL << carry_name << " = this->registers.get(Flags::Carry) ? 0x80U : 0U;\n"
				<< TEMPDECL << result_name << " = (" << temp_to_string(val) << " >> 1) | " << carry_name << ";\n";
		}else{
			auto bit_name = get_temp_name(n++);
			s
				<< TEMPDECL << bit_name << " = " << temp_to_string(val) << " & 0xFF;\n"
				<< TEMPDECL << result_name << " = (" << bit_name << " >> 1) | (" << bit_name << " << 7);\n";
		}
	}

	s << "\t" << result_name << " = " << result_name << " & 0xFF;\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::stop(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->stop();\n";
}

uintptr_t InterpreterCodeGenerator::shift8_left(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << TEMPDECL << result_name << " = " << temp_to_string(val) << " << 1;\n";

	s << "\t" << result_name << " = " << result_name << " & 0xFF;\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::arithmetic_shift_right(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto temp_name = get_temp_name(n++);

	s
		<< TEMPDECL << temp_name << " = " << temp_to_string(val) << " & 0xFF;\n"
		<< TEMPDECL << result_name << " = (" << temp_name << " & 0x80) | (" << temp_name << " >> 1);\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::bitwise_shift_right(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << TEMPDECL << result_name << " = (" << temp_to_string(val) << " & 0xFF) >> 1;\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_bit_value(uintptr_t val, unsigned bit){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << TEMPDECL << result_name << " = " << temp_to_string(val) << " & " << (1 << bit) << ";\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::set_bit_value(uintptr_t val, unsigned bit, bool on){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto mask_name = get_temp_name(n++);

	s
		<< CONSTTEMPDECL << mask_name << " = " << (1 << bit) << ";\n"
		<< TEMPDECL << result_name << " = " << temp_to_string(val);
	if (!on)
		s << " & ~";
	else
		s << " | ";
	s << mask_name << ";\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

std::pair<uintptr_t, uintptr_t> InterpreterCodeGenerator::perform_decimal_adjustment(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto carry_name = get_temp_name(n++);

	s
		<< TEMPDECL << result_name << " = this->decimal_adjust(" << temp_to_string(val) << ");\n"
		<< TEMPDECL << carry_name << " = " << result_name << " & 0x100;\n";

	auto ret0 = copy(result_name);
	auto ret1 = copy(carry_name);
	this->temporary_values.push_back(ret0);
	this->temporary_values.push_back(ret1);
	return { (uintptr_t)ret0, (uintptr_t)ret1 };
}

uintptr_t InterpreterCodeGenerator::swap_nibbles(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);
	auto temp_name = get_temp_name(n++);

	s
		<< TEMPDECL << temp_name << " = " << temp_to_string(val) << " & 0xFF;\n"
		<< TEMPDECL << result_name << " = (" << temp_name << "<< 4) | (" << temp_name << " >> 4);\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

uintptr_t InterpreterCodeGenerator::get_imm_value(unsigned val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << CONSTTEMPDECL << result_name << " = " << val << ";\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::require_equals(uintptr_t a, uintptr_t b){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;

	s
		<< "\tif (" << temp_to_string(a) << " != " << temp_to_string(b) << ")\n"
		<< "\t\tthis->abort();\n";
}

void InterpreterCodeGenerator::do_nothing_if(uintptr_t val, unsigned take_time, bool invert){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;

	s << "\tif (";
	if (invert)
		s << "!";
	s
		<< temp_to_string(val) << "){\n"
		<< "\t\tthis->take_time(" << take_time << ");\n"
		<< "\t\treturn;\n"
		<< "\t}\n";
}

uintptr_t InterpreterCodeGenerator::condition_to_value(ConditionalJumpType type){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	switch (type){
		case ConditionalJumpType::NotZero:
			s << TEMPDECL << result_name << " = !this->registers.get(Flags::Zero);\n";
			break;
		case ConditionalJumpType::Zero:
			s << TEMPDECL << result_name << " = this->registers.get(Flags::Zero);\n";
			break;
		case ConditionalJumpType::NotCarry:
			s << TEMPDECL << result_name << " = !this->registers.get(Flags::Carry);\n";
			break;
		case ConditionalJumpType::Carry:
			s << TEMPDECL << result_name << " = this->registers.get(Flags::Carry);\n";
			break;
		default: break;


	}

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}

void InterpreterCodeGenerator::abort(){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	s << "\tthis->abort();\n";
}

uintptr_t InterpreterCodeGenerator::sign_extend8(uintptr_t val){
	auto &back = this->definition_stack.back();
	auto &s = *back.function_contents;
	auto &n = back.temporary_index;
	auto result_name = get_temp_name(n++);

	s << TEMPDECL << result_name << " = sign_extend8<" TEMPTYPE ">(" << temp_to_string(val) << ");\n";

	auto ret = copy(result_name);
	this->temporary_values.push_back(ret);
	return (uintptr_t)ret;
}
