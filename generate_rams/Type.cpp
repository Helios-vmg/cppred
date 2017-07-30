#include "Type.h"
#include <boost/format.hpp>
#include <utility>
#include <map>
#include <cctype>

typedef std::unique_ptr<Type> (*basic_type_constructor)();

std::unique_ptr<Type> construct_fail(){
	throw std::exception();
}

std::unique_ptr<Type> construct_u8(){
	return std::make_unique<TypeUint>(1);
}

std::unique_ptr<Type> construct_u16(){
	return std::make_unique<TypeUint>(2);
}

std::unique_ptr<Type> construct_u24(){
	return std::make_unique<TypeUint>(3);
}

std::unique_ptr<Type> construct_big_u16(){
	return std::make_unique<TypeUint>(2, true);
}

std::unique_ptr<Type> construct_big_u24(){
	return std::make_unique<TypeUint>(3, true);
}

std::unique_ptr<Type> construct_big_u32(){
	return std::make_unique<TypeUint>(4, true);
}

std::unique_ptr<Type> construct_spritestatedata1(){
	return std::make_unique<spritestatedata1Struct>();
}

std::unique_ptr<Type> construct_spritestatedata2(){
	return std::make_unique<spritestatedata2Struct>();
}

std::unique_ptr<Type> construct_pointer(){
	return std::make_unique<DataPointer>();
}

std::unique_ptr<Type> construct_big_pointer(){
	return std::make_unique<DataPointer>();
}

std::unique_ptr<Type> construct_code_pointer(){
	return std::make_unique<CodePointer>();
}

std::unique_ptr<Type> construct_bcd4(){
	return std::make_unique<TypeBcdInt>(4);
}

std::unique_ptr<Type> construct_bcd6(){
	return std::make_unique<TypeBcdInt>(6);
}

#define ENUM_TYPE(x) construct_##x

#define DEFINE_construct_enum_byte(x) \
std::unique_ptr<Type> ENUM_TYPE(x)(){ \
	return std::make_unique<EnumUint>(#x, 1); \
}

DEFINE_construct_enum_byte(SerialConnectionStatus);
DEFINE_construct_enum_byte(SpeciesId);
DEFINE_construct_enum_byte(Sound);
DEFINE_construct_enum_byte(AudioBank);

#define DECLARE_ENUM_IN_MAP(x) { #x, ENUM_TYPE(x) }

const std::map<std::string, basic_type_constructor> normal_types = {
	{ "u8", construct_u8 },
	{ "u16", construct_u16 },
	{ "u24", construct_u24 },
	{ "big_u16", construct_big_u16 },
	{ "big_u24", construct_big_u32 },
	{ "big_u32", construct_big_u32 },
	{ "spritestatedata1", construct_spritestatedata1 },
	{ "spritestatedata2", construct_spritestatedata2 },
	{ "pointer", construct_pointer },
	{ "big_pointer", construct_big_pointer },
	{ "code_pointer", construct_code_pointer },
	{ "bcd4", construct_bcd4 },
	{ "bcd6", construct_bcd6 },
	DECLARE_ENUM_IN_MAP(SerialConnectionStatus),
	DECLARE_ENUM_IN_MAP(SpeciesId),
	DECLARE_ENUM_IN_MAP(Sound),
	DECLARE_ENUM_IN_MAP(AudioBank),
};

bool is_number(const std::string &s){
	return std::all_of(s.begin(), s.end(), isdigit);
}

std::unique_ptr<Type> parse_string_to_Type(const std::string &s){
	auto it = normal_types.find(s);
	if (it != normal_types.end())
		return it->second();
	
	if (s.back() != ']')
		throw std::runtime_error("Unrecognized type string: " + s);
	auto bracket = s.rfind('[');
	if (bracket == s.npos)
		throw std::runtime_error("Unrecognized type string: " + s);
	auto subtype_string = s.substr(0, bracket);
	bracket++;
	auto length_string = s.substr(bracket, s.size() - 1 - bracket);
	std::unique_ptr<Number> length;
	if (is_number(length_string))
		length.reset(new IntegerLiteralNumber(length_string));
	else
		length.reset(new CompoundNumber(length_string));
	auto subtype = parse_string_to_Type(subtype_string);
	return std::make_unique<Array>(std::move(subtype), std::move(length));
}

const char * const integer_types[] = {
	"byte_t",
	"std::uint16_t",
	"std::uint32_t",
	"std::uint32_t",
};

const char * const integer_functions[2][4][2] = {
	{
		{"read_memory_u8",	"write_memory_u8", },
		{"read_memory_u16", "write_memory_u16", },
		{"read_memory_u24", "write_memory_u24", },
		{"read_memory_u32", "write_memory_u32", },
	},
	{
		{"read_memory_u8_big",	"write_memory_u8_big", },
		{"read_memory_u16_big", "write_memory_u16_big", },
		{"read_memory_u24_big", "write_memory_u24_big", },
		{"read_memory_u32_big", "write_memory_u32_big", },
	},
};

const char * const bcd_types[] = {
	"IntegerWrapper<unsigned, 1>",
	"IntegerWrapper<unsigned, 2>",
	"IntegerWrapper<unsigned, 3>",
	"IntegerWrapper<unsigned, 4>",
};

std::string Type::generate_declarations(const std::string &name) const{
	return this->get_actual_type_name() + " " + name + ";\n";
}

std::string Type::generate_initializer(unsigned address, unsigned base_address, const std::unique_ptr<Number> &size, const std::string &name) const{
	return (boost::format(
			"%1%(this->memory + %2%, %3%)"
		)
		% name
		% (address - base_address)
		% this->get_callback_struct()
	).str();
}

std::string TypeUint::get_actual_type_name() const{
	return (boost::format("IntegerWrapper<%1%, %2%>") % this->get_virtual_type_name() % this->N).str();
}

std::string TypeUint::get_virtual_type_name() const{
	return integer_types[this->N - 1];
}

std::string TypeBcdInt::get_actual_type_name() const{
	return (boost::format("IntegerWrapper<%1%, %2%>") % this->get_virtual_type_name() % (this->N / 2)).str();
}

std::string Pointer::get_actual_type_name() const{
	return (boost::format("IntegerWrapper<%1%, 2>") % this->get_virtual_type_name()).str();
}

std::string Pointer::get_virtual_type_name() const{
	return "std::uint16_t";
}

std::string spritestatedata1Struct::get_actual_type_name() const{
	return "SpriteStateData1";
}

std::string spritestatedata1Struct::get_callback_struct() const{
	return (boost::format(
			"{%1%, %2%}"
		)
		% integer_functions[0][0][0]
		% integer_functions[0][0][1]
	).str();
}

std::string spritestatedata2Struct::get_callback_struct() const{
	return (boost::format(
			"{%1%, %2%}"
		)
		% integer_functions[0][0][0]
		% integer_functions[0][0][1]
	).str();
}

std::string spritestatedata2Struct::get_actual_type_name() const{
	return "SpriteStateData2";
}

std::string Array::get_actual_type_name() const{
#if 0
	return (boost::format(
			"WrappedArray<typename %1%::type, %2%, %3%>"
		)
		% this->inner->get_actual_type_name()
		% this->length->to_string()
		% this->inner->get_size()->to_string()
	).str();
#else
	return (boost::format(
			"WrappedArray<%1%, %2%, %3%>"
		)
		% this->inner->get_virtual_type_name()
		% this->length->to_string()
		% this->inner->get_size()->to_string()
	).str();
#endif
}

std::string TypeUint::get_callback_struct() const{
	return (boost::format(
			"{%1%, %2%}"
		)
		% integer_functions[(int)this->big_endian][this->N - 1][0]
		% integer_functions[(int)this->big_endian][this->N - 1][1]
	).str();
}

std::string TypeBcdInt::get_callback_struct() const{
	return (boost::format("{read_bcd%1%, write_bcd%1%}") % this->N).str();
}

std::string Struct::generate_initializer(unsigned address, unsigned base_address, const std::unique_ptr<Number> &size, const std::string &name) const{
	return (boost::format(
			"%1%(this->memory + %2%, %3%)"
		)
		% name
		% (address - base_address)
		% this->get_callback_struct()
	).str();
}

std::string Pointer::get_callback_struct() const{
	return (boost::format(
			"{%1%, %2%}"
		)
		% integer_functions[(int)this->big_endian][1][0]
		% integer_functions[(int)this->big_endian][1][1]
	).str();
}

std::string EnumUint::get_actual_type_name() const{
	return (boost::format("EnumWrapper<%1%, %2%, %3%>") % this->type_name % TypeUint::get_virtual_type_name() % this->N).str();
}
