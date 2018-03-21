#include "generate_variables.h"
#include "Variables.h"

static const char * const map_objects_file = "input/events.csv";
static const char * const map_sprites_visibility_file = "input/map_sprites_visibility.csv";
extern const char * const variables_file = "input/variables.csv";
static const char * const hash_key = "generate_variables";
static const char * const date_string = __DATE__ __TIME__;

static void generate_file(std::ostream &stream, const char *enum_name, const char *input_filename, const char *id_column, const char *name_column){
	CsvParser csv(input_filename);
	auto rows = csv.row_count();

	const std::vector<std::string> order = {
		id_column,
		name_column,
	};
	
	stream <<
		"enum class " << enum_name << "{\n"
		"    None = 0,\n";

	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, order);
		auto id = to_unsigned(columns[0]);
		stream << "    " << columns[1] << " = " << id << ",\n";
	}
	stream << "};\n\n";
}

static std::vector<byte_t> load_default_visibilities(){
	static const std::vector<std::string> order = {"id", "initial_visibility_state"};
	std::vector<byte_t> ret;
	CsvParser csv(map_sprites_visibility_file);
	auto rows = csv.row_count();
	std::string column = "id";
	for (auto i = rows; i--;){
		auto bit = to_unsigned(csv.get_cell(i, column)) - 1;
		if (ret.size() <= bit / 8)
			ret.resize(bit / 8 + 1, 0xFF);
	}
	for (auto i = rows; i--;){
		auto columns = csv.get_ordered_row(i, order);
		auto initial_visibility_state = to_bool(columns[1]);
		if (initial_visibility_state)
			continue;
		auto index = to_unsigned(columns[0]) - 1;
		auto bit = index % 8;
		auto byte = index / 8;
		ret[byte] ^= 1 << bit;
	}
	return ret;
}

static void generate_variables_internal(known_hashes_t &known_hashes, Variables &variables){
	std::vector<std::string> input_files = {
		map_objects_file,
		map_sprites_visibility_file,
		variables_file,
	};
	auto current_hash = hash_files(input_files, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating variables.\n";
		return;
	}
	std::cout << "Generating variables...\n";
	
	std::ofstream header("output/variables.h");
	std::ofstream source("output/variables.inl");

	header << generated_file_warning <<
		"\n"
		"#pragma once\n"
		"\n"
		"namespace CppRed{\n";

	source << generated_file_warning <<
		"\n"
		"namespace CppRed{\n";


	generate_file(header, "EventId", map_objects_file, "id", "name");
	generate_file(header, "VisibilityFlagId", map_sprites_visibility_file, "id", "visibility_flag");

	std::vector<std::string> integers;
	std::vector<std::string> strings;
	for (auto &kv : variables.get_map()){
		auto &dst = kv.second.is_string ? strings : integers;
		if (dst.size() <= kv.second.id)
			dst.resize(kv.second.id + 1);
		dst[kv.second.id] = kv.first;
	}

	auto default_visibilities = load_default_visibilities();
	
	header << "enum class IntegerVariableId{\n";
	unsigned i = 0;
	for (auto &name : integers)
		header << "    " << name << " = " << i++ << ",\n";
	header <<
		"};\n"
		"\n"
		"enum class StringVariableId{\n";
	i = 0;
	for (auto &name : strings)
		header << "    " << name << " = " << i++ << ",\n";

	header << "};\n";

	write_buffer_to_header_and_source(header, source, default_visibilities, "default_sprite_vibisilities");
	header << "}\n";
	source << "}\n";

	known_hashes[hash_key] = current_hash;
}

void generate_variables(known_hashes_t &known_hashes, Variables &variables){
	try{
		generate_variables_internal(known_hashes, variables);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_variables(): " + e.what());
	}
}
