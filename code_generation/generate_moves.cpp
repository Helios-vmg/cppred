#include "generate_moves.h"
#include <iostream>
#include <algorithm>

static const char * const input_file = "input/moves.csv";
static const char * const hash_key = "generate_moves";
static const char * const date_string = __DATE__ __TIME__;

struct MoveData{
	unsigned id;
	std::string name;
	unsigned field_move_index;
	std::string display_name;
};

static void generate_enums(const std::map<unsigned, MoveData> &moves){
	std::ofstream move_enums("output/move_enums.h");
	move_enums << generated_file_warning <<
		"\n"
		"enum class MoveId{\n";

	for (auto &kv : moves)
		move_enums << "    " << kv.second.name << " = " << kv.second.id << ",\n";
	
	move_enums << "};\n";
}

static void generate_declarations(const std::map<unsigned, MoveData> &moves, const std::vector<MoveData *> &field_moves){
	std::ofstream move_declarations("output/move_data.h");
	move_declarations << generated_file_warning <<
		"\n";

	for (auto &kv : moves)
		move_declarations << "extern const MoveInfo moveinfo_" << kv.second.name << ";\n";

	move_declarations << "extern const MoveInfo * const moves_by_move_id[" << moves.size() << "];\n";

#if 0
	move_declarations << "extern const MoveInfo * const field_moves_by_field_move_index[" << field_moves.size() << "];\n";
#endif
}

static void generate_definitions(const std::map<unsigned, MoveData> &moves, const std::vector<MoveData *> &field_moves){
	std::ofstream move_definitions("output/move_data.inl");
	move_definitions << generated_file_warning <<
		"\n";

	for (auto &kv : moves)
		move_definitions <<
		"const MoveInfo moveinfo_" << kv.second.name << " = {\n"
		"    MoveId::" << kv.second.name << ",\n"
		"    " << kv.second.field_move_index << ",\n"
		"    \"" << kv.second.display_name << "\"\n"
		"};\n\n";

	move_definitions << "const MoveInfo * const moves_by_move_id[" << moves.size() << "] = {\n";
	for (auto &kv : moves)
		move_definitions << "    &moveinfo_" << kv.second.name << ",\n";
	move_definitions <<
		"};\n";

#if 0
	move_definitions <<
		"\n"
		"const MoveInfo * const field_moves_by_field_move_index[" << field_moves.size() << "] = {\n";
	for (auto &fm : field_moves)
		move_definitions << "    &moveinfo_" << fm->name << ",\n";
	move_definitions << "};\n";
#endif
}

static void generate_moves_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating moves.\n";
		return;
	}
	std::cout << "Generating moves...\n";

	static const std::vector<std::string> data_order = {
		"id",
		"name",
		"field_move_index",
		"display_name",
	};

	CsvParser csv(input_file);
	auto rows = csv.row_count();
	std::map<unsigned, MoveData> moves;
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, data_order);
		auto id = to_unsigned(row[0]);
		auto name = row[1];
		auto field_move_index = to_unsigned(row[2]);
		auto display_name = row[3];
		moves[id] = { id, name, field_move_index, display_name };
	}
	
	std::vector<MoveData *> field_moves;
	for (auto &kv : moves){
		if (!kv.second.field_move_index)
			continue;
		field_moves.push_back(&kv.second);
	}
	std::sort(field_moves.begin(), field_moves.end(), [](MoveData *a, MoveData *b){ return a->field_move_index < b->field_move_index; });

	generate_enums(moves);
	generate_declarations(moves, field_moves);
	generate_definitions(moves, field_moves);

	known_hashes[hash_key] = current_hash;
}

void generate_moves(known_hashes_t &known_hashes){
	try{
		generate_moves_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_moves(): " + e.what());
	}
}
