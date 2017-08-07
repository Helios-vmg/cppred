#include "../common/csv_parser.h"
#include <vector>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>

const std::vector<std::string> data_order = {
	"species_id",      // 0
	"pokedex_id",      // 1
	"name",            // 2
	"base_hp",         // 3
	"base_attack",     // 4
	"base_defense",    // 5
	"base_speed",      // 6
	"base_special",    // 7
	"type1",           // 8
	"type2",           // 9
	"catch_rate",      // 10
	"base_xp_yield",   // 11
	"initial_attack1", // 12
	"initial_attack2", // 13
	"initial_attack3", // 14
	"initial_attack4", // 15
	"growth_rate",     // 16
	"tmlearn_bitmap",  // 17
	"display_name",    // 18
	"front_image",     // 19
	"back_image",      // 20
	"cry_base",        // 21
	"cry_pitch",       // 22
	"cry_length",      // 23
};

const std::vector<std::string> evolutions_order = {
	"species",       // 0
	"type",          // 1
	"minimum_level", // 2
	"next_form",     // 3
	"item",          // 4
};

const std::vector<std::string> moves_order = {
	"species", // 0
	"level",   // 1
	"move",    // 2
};

unsigned to_unsigned(const std::string &s){
	std::stringstream stream(s);
	unsigned ret;
	if (!(stream >> ret))
		throw std::runtime_error("Can't convert \"" + s + "\" to integer.");
	return ret;
}

unsigned to_unsigned_default(const std::string &s, unsigned def = 0){
	try{
		return to_unsigned(s);
	}catch (std::exception &){
		return def;
	}
}

unsigned hex_no_prefix_to_integer(const std::string &s){
	std::stringstream stream;
	stream << std::hex << s;
	unsigned ret;
	if (!(stream >> ret))
		throw std::runtime_error((std::string)"Can't convert \"" + s + "\" from hex to integer.");
	return ret;
}

unsigned hex_no_prefix_to_integer_default(const std::string &s, unsigned def = 0){
	try{
		return hex_no_prefix_to_integer(s);
	}catch (std::exception &){
		return def;
	}
}

class EvolutionTrigger{
public:
	std::string species;
	std::string type;
	unsigned minimum_level;
	std::string next_form;
	std::string item;

	EvolutionTrigger(const std::vector<std::string> &columns);
};

class LearnedMove{
public:
	std::string species;
	unsigned level;
	std::string move;

	LearnedMove(const std::vector<std::string> &columns);
};

class SpeciesData{
public:
	unsigned species_id;
	unsigned pokedex_id;
	std::string name;
	unsigned base_hp;
	unsigned base_attack;
	unsigned base_defense;
	unsigned base_speed;
	unsigned base_special;
	std::string type[2];
	unsigned catch_rate;
	unsigned base_xp_yield;
	std::vector<std::string> initial_attacks;
	unsigned growth_rate;
	unsigned char tmlearn_bitmap[7];
	std::string display_name;
	std::string front_image;
	std::string back_image;
	unsigned cry_base;
	unsigned cry_pitch;
	unsigned cry_length;
	std::vector<EvolutionTrigger> evolution_triggers;
	std::vector<LearnedMove> learned_moves;

	SpeciesData(const std::vector<std::string> &columns);

	bool operator<(const SpeciesData &other) const{
		if ((!this->pokedex_id && !this->species_id) && !(!other.pokedex_id && !other.species_id))
			return true;
		if (!(!this->pokedex_id && !this->species_id) && (!other.pokedex_id && !other.species_id))
			return false;
		if (this->pokedex_id && !other.pokedex_id)
			return true;
		if (!this->pokedex_id && other.pokedex_id)
			return false;
		if (this->pokedex_id < other.pokedex_id)
			return true;
		if (this->pokedex_id > other.pokedex_id)
			return false;
		return this->species_id < other.species_id;
	}
};

class PokemonData{
private:
	std::vector<SpeciesData> species;

	unsigned count_pokedex_species() const;
public:
	PokemonData();
	void generate_enums(const char *filename) const;
	void generate_static_data_declarations(const char *filename) const;
	void generate_static_data_definitions(const char *filename, const char *header_name) const;
};

PokemonData::PokemonData(){
	{
		CsvParser csv("input/pokemon_data.csv");
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++)
			this->species.push_back(SpeciesData(csv.get_ordered_row(i, data_order)));
	}

	std::map<std::string, SpeciesData *> species;
	for (auto &s : this->species)
		species[s.name] = &s;

	{
		CsvParser csv("input/evolutions.csv");
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++){
			EvolutionTrigger evolution(csv.get_ordered_row(i, evolutions_order));
			species[evolution.species]->evolution_triggers.push_back(evolution);
		}
	}
	{
		CsvParser csv("input/moves.csv");
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++){
			LearnedMove move(csv.get_ordered_row(i, moves_order));
			species[move.species]->learned_moves.push_back(move);
		}
	}

	std::sort(this->species.begin(), this->species.end());
}

SpeciesData::SpeciesData(const std::vector<std::string> &columns){
	this->species_id = to_unsigned(columns[0]);
	this->pokedex_id = to_unsigned_default(columns[1]);
	this->name = columns[2];
	this->base_hp = to_unsigned_default(columns[3]);
	this->base_attack = to_unsigned_default(columns[4]);
	this->base_defense = to_unsigned_default(columns[5]);
	this->base_speed = to_unsigned_default(columns[6]);
	this->base_special = to_unsigned_default(columns[7]);
	for (int i = 0; i < 2; i++){
		this->type[i] = columns[8 + i];
		if (!this->type[i].size())
			this->type[i] = "Normal";
	}
	this->catch_rate = to_unsigned_default(columns[10]);
	this->base_xp_yield = to_unsigned_default(columns[11]);
	for (int i = 0; i < 4; i++){
		if (!columns[12 + i].size())
			continue;
		this->initial_attacks.push_back(columns[12 + i]);
	}
	this->growth_rate = to_unsigned_default(columns[16]);
	{
		auto &bitmap = columns[17];
		if (bitmap.size() != 7 * 8)
			throw std::runtime_error("tmlearn_bitmap has invalid length.");
		for (int i = 0; i < 7; i++){
			this->tmlearn_bitmap[i] = 0;
			for (int j = 0; j < 8; j++){
				char c = bitmap[i * 8 + j];
				this->tmlearn_bitmap[i] |= (c != '0') << j;
			}
		}
	}
	this->display_name = columns[18];
	this->front_image = columns[19];
	this->back_image = columns[20];
	this->cry_base = hex_no_prefix_to_integer_default(columns[21]);
	this->cry_pitch = hex_no_prefix_to_integer_default(columns[22]);
	this->cry_length = hex_no_prefix_to_integer_default(columns[23]);

	if (this->front_image.size())
		this->front_image = "&" + this->front_image;
	else
		this->front_image = "nullptr";
	if (this->back_image.size())
		this->back_image = "&" + this->back_image;
	else
		this->back_image = "nullptr";
}

EvolutionTrigger::EvolutionTrigger(const std::vector<std::string> &columns){
	this->species = columns[0];
	this->type = columns[1];
	this->minimum_level = to_unsigned(columns[2]);
	this->next_form = columns[3];
	this->item = columns[4];
}

LearnedMove::LearnedMove(const std::vector<std::string> &columns){
	this->species = columns[0];
	this->level = to_unsigned(columns[1]);
	this->move = columns[2];
}

void PokemonData::generate_enums(const char *filename) const{
	std::ofstream file(filename);

	file << 
		"#pragma once\n"
		"\n"
		"//This file is autogenerated. Do not edit.\n"
		"\n"
		"enum class SpeciesId{\n";
	{
		std::vector<std::pair<unsigned, std::string>> temp;
		for (auto &species : this->species)
			temp.push_back({species.species_id, species.name});
		std::sort(temp.begin(), temp.end());
		for (auto &species : temp)
			file << "    " << species.second << " = " << species.first << ",\n";
	}
	file <<
		"};\n"
		"\n"
		"enum class PokedexId{\n";
	for (auto &species : this->species){
		if (!species.pokedex_id && species.species_id)
			break;
		file << "    " << species.name << " = " << species.pokedex_id << ",\n";
	}
	file << "};\n";
}

void PokemonData::generate_static_data_declarations(const char *filename) const{
	std::ofstream file(filename);

	file <<
		"#pragma once\n"
		"\n"
		"//This file is autogenerated. Do not edit.\n"
		"\n"
		"#include \"../../cppred/CppRedStructs.h\"\n"
		"#include \"gfx.h\"\n"
		"\n";

	for (auto &species : this->species)
		file << "extern const PokemonInfo<" << species.initial_attacks.size() << ", " << species.evolution_triggers.size() << ", " << species.learned_moves.size() << "> info_" << species.name << ";\n";

	file <<
		"extern const BasePokemonInfo * const pokemon_by_species_id[" << this->species.size() << "];\n"
		"extern const BasePokemonInfo * const pokemon_by_pokedex_id[" << this->count_pokedex_species() << "];\n";
}

void PokemonData::generate_static_data_definitions(const char *filename, const char *header_name) const{
	std::ofstream file(filename);

	file <<
		"//This file is autogenerated. Do not edit.\n"
		"\n"
		"#include \"" << header_name << "\"\n"
		"\n";

	for (auto &species : this->species){
		file <<
			"const PokemonInfo<" << species.initial_attacks.size() << ", " << species.evolution_triggers.size() << ", " << species.learned_moves.size() << "> info_" << species.name << " = {\n"
			"    PokedexId::" << (species.pokedex_id ? species.name : "None") << ",\n"
			"    SpeciesId::" << species.name << ",\n"
			"    " << species.base_hp << ",\n"
			"    " << species.base_attack << ",\n"
			"    " << species.base_defense << ",\n"
			"    " << species.base_speed << ",\n"
			"    " << species.base_special << ",\n"
			"    {\n"
			"        PokemonTypeId::" << species.type[0] << ",\n"
			"        PokemonTypeId::" << species.type[1] << ",\n"
			"    },\n"
			"    " << species.catch_rate << ",\n"
			"    " << species.base_xp_yield << ",\n"
			"    " << species.growth_rate << ",\n"
			"    { " << std::hex
		;
		for (auto &tm : species.tmlearn_bitmap)
			file << "0x" << std::setw(2) << std::setfill('0') << (int)tm << ", ";
		file << std::dec <<
			"},\n"
			"    \"" << species.display_name << "\",\n"
			"    " << species.front_image << ",\n"
			"    " << species.back_image << ",\n"
			"    { " << std::hex <<
			"0x" << std::setw(2) << std::setfill('0') << species.cry_base << ", "
			"0x" << std::setw(2) << std::setfill('0') << species.cry_pitch << ", "
			"0x" << std::setw(2) << std::setfill('0') << species.cry_length << std::dec <<
			" },\n"
			"    {\n"
			;

		for (auto &attack : species.initial_attacks)
			file << "        MoveId::" << attack << ",\n";

		file <<
			"    },\n"
			"    {\n"
		;

		for (auto &evolution : species.evolution_triggers){
			file << "        ";
			const char *type;
			if (evolution.type == "level")
				type = "AT_LEVEL";
			else if (evolution.type == "item")
				type = "WITH_ITEM";
			else if (evolution.type == "trade")
				type = "WHEN_TRADED";
			else
				throw std::runtime_error("Invalid type: " + evolution.type);
			file << type << "(";
			if (evolution.type == "item")
				file << evolution.item << ", ";
			file << evolution.minimum_level << ", " << evolution.next_form << "),\n";
		}
		file <<
			"    },\n"
			"    {\n";
		for (auto &move : species.learned_moves)
			file << "        LEARN(" << move.level << ", " << move.move << "),\n";
		file <<
			"    },\n"
			"};\n"
			"\n";
	}

	{
		std::vector<std::pair<unsigned, std::string>> temp;
		for (auto &species : this->species)
			temp.push_back({ species.species_id, species.name });
		std::sort(temp.begin(), temp.end());
		file << "const BasePokemonInfo * const pokemon_by_species_id[" << temp.size() << "] = {\n";
		for (auto &species : temp)
			file << "    &info_" << species.second << ",\n";
		file <<
			"};\n"
			"\n";
	}
	{
		file << "const BasePokemonInfo * const pokemon_by_pokedex_id[" << this->count_pokedex_species() << "] = {\n";
		for (auto &species : this->species){
			if (!species.pokedex_id)
				continue;
			file << "    &info_" << species.name << ",\n";
		}
		file <<
			"};\n"
			"\n";
	}
}

unsigned PokemonData::count_pokedex_species() const{
	unsigned ret = 0;
	for (auto &species : this->species)
		ret += !!species.pokedex_id;
	return ret;
}

int main(){
	try{
		PokemonData data;
		data.generate_enums("output/pokemon_enums.h");
		data.generate_static_data_declarations("output/pokemon_declarations.h");
		data.generate_static_data_definitions("output/pokemon_definitions.cpp", "pokemon_declarations.h");
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
		return -1;
	}
	return 0;
}
