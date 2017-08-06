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
	"pokedex_id",	   // 1
	"name",			   // 2
	"base_hp",		   // 3
	"base_attack",	   // 4
	"base_defense",	   // 5
	"base_speed",	   // 6
	"base_special",	   // 7
	"type1",		   // 8
	"type2",		   // 9
	"catch_rate",	   // 10
	"base_xp_yield",   // 11
	"initial_attack1", // 12
	"initial_attack2", // 13
	"initial_attack3", // 14
	"initial_attack4", // 15
	"growth_rate",	   // 16
	"tmlearn_bitmap",  // 17
	"display_name",	   // 18
	"front_image",	   // 19
	"back_image",	   // 20
	"cry_base",		   // 21
	"cry_pitch",	   // 22
	"cry_length",	   // 23
};

const std::vector<std::string> evolutions_order = {
	"species_id",			  // 0
	"type",					  // 1
	"minimum_level",		  // 2
	"next_form_species_id",	  // 3
	"next_form_species_name", // 4
	"item_id",				  // 5
	"item_name",			  // 6
};

const std::vector<std::string> moves_order = {
	"species_id", // 0
	"at_level",   // 1
	"move_id",    // 2
	"move_name",  // 3
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
	unsigned species_id;
	std::string type;
	unsigned minimum_level;
	unsigned next_form_species_id;
	std::string next_form_species_name;
	unsigned item_id;
	std::string item_name;

	EvolutionTrigger(const std::vector<std::string> &columns);
};

class LearnedMove{
public:
	unsigned species_id;
	unsigned at_level;
	unsigned move_id;
	std::string move_name;

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
	std::string initial_attacks[4];
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

	unsigned get_max_evolutions() const;
	unsigned get_max_learned_moves() const;
	unsigned count_pokedex_species() const;
public:
	PokemonData();
	void generate_enums(const char *filename) const;
	void generate_array_requirements(const char *filename) const;
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

	std::map<unsigned, SpeciesData *> species;
	for (auto &s : this->species)
		species[s.species_id] = &s;

	{
		CsvParser csv("input/evolutions.csv");
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++){
			EvolutionTrigger evolution(csv.get_ordered_row(i, evolutions_order));
			species[evolution.species_id]->evolution_triggers.push_back(evolution);
		}
	}
	{
		CsvParser csv("input/moves.csv");
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++){
			LearnedMove move(csv.get_ordered_row(i, moves_order));
			species[move.species_id]->learned_moves.push_back(move);
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
		this->initial_attacks[i] = columns[12 + i];
		if (!this->initial_attacks[i].size())
			this->initial_attacks[i] = "None";
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
	this->species_id = to_unsigned(columns[0]);
	this->type = columns[1];
	this->minimum_level = to_unsigned(columns[2]);
	this->next_form_species_id = to_unsigned(columns[3]);
	this->next_form_species_name = columns[4];
	this->item_id = to_unsigned_default(columns[5]);
	this->item_name = columns[6];
}

LearnedMove::LearnedMove(const std::vector<std::string> &columns){
	this->species_id = to_unsigned(columns[0]);
	this->at_level = to_unsigned(columns[1]);
	this->move_id = to_unsigned(columns[2]);
	this->move_name = columns[3];
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

void PokemonData::generate_array_requirements(const char *filename) const{
	std::ofstream file(filename);

	file << 
		"#pragma once\n"
		"\n"
		"//This file is autogenerated. Do not edit.\n"
		"\n"
		"const unsigned max_evolution_triggers = " << this->get_max_evolutions() << ";\n"
		"const unsigned max_learned_moves = " << this->get_max_learned_moves() << ";\n";
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
		file << "extern const PokemonInfo info_" << species.name << ";\n";

	file <<
		"extern const PokemonInfo * const pokemon_by_species_id[" << this->species.size() << "];\n"
		"extern const PokemonInfo * const pokemon_by_pokedex_id[" << this->count_pokedex_species() << "];\n";
}

void PokemonData::generate_static_data_definitions(const char *filename, const char *header_name) const{
	std::ofstream file(filename);

	auto max_evolutions = this->get_max_evolutions();
	auto max_moves = this->get_max_learned_moves();

	file <<
		"//This file is autogenerated. Do not edit.\n"
		"\n"
		"#include \"" << header_name << "\"\n"
		"\n";

	for (auto &species : this->species){
		file <<
			"const PokemonInfo info_" << species.name << " = {\n"
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
			"    {\n"
		;

		for (auto &attack : species.initial_attacks)
			file << "        MoveId::" << attack << ",\n";
		file <<
			"    },\n"
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
			"    {" << std::hex <<
			"0x" << std::setw(2) << std::setfill('0') << species.cry_base << ", "
			"0x" << std::setw(2) << std::setfill('0') << species.cry_pitch << ", "
			"0x" << std::setw(2) << std::setfill('0') << species.cry_length << ", " << std::dec <<
			"},\n"
			"    {\n"
		;

		for (auto &evolution : species.evolution_triggers){
			file <<
				"        {\n"
				"            ";
			const char *type;
			if (evolution.type == "level")
				type = "EvolutionTriggerType::Level";
			else if (evolution.type == "item")
				type = "EvolutionTriggerType::Item";
			else if (evolution.type == "trade")
				type = "EvolutionTriggerType::Trade";
			else
				type = "EvolutionTriggerType::None";
			file <<
				type << ",\n"
				"            " << evolution.minimum_level << ",\n"
				"            SpeciesId::" << evolution.next_form_species_name << ",\n"
				"            ItemId::";
			if (evolution.type == "item")
				file << evolution.item_name;
			else
				file << "None";
			file <<
				",\n"
				"        },\n";
		}
		file <<
			"    },\n"
			"    {\n";
		for (auto &move : species.learned_moves)
			file << "        { " << move.at_level << ", MoveId::" << move.move_name << " },\n";
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
		file << "const PokemonInfo * const pokemon_by_species_id[" << temp.size() << "] = {\n";
		for (auto &species : temp)
			file << "    &info_" << species.second << ",\n";
		file <<
			"};\n"
			"\n";
	}
	{
		file << "const PokemonInfo * const pokemon_by_pokedex_id[" << this->count_pokedex_species() << "] = {\n";
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

unsigned PokemonData::get_max_evolutions() const{
	unsigned max = 0;
	for (auto &species : this->species)
		max = std::max(max, species.evolution_triggers.size());
	return max;
}

unsigned PokemonData::get_max_learned_moves() const{
	unsigned max = 0;
	for (auto &species : this->species)
		max = std::max(max, species.learned_moves.size());
	return max;
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
		data.generate_array_requirements("output/pokemon_array_sizes.h");
		data.generate_static_data_declarations("output/pokemon_declarations.h");
		data.generate_static_data_definitions("output/pokemon_definitions.cpp", "pokemon_declarations.h");
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
		return -1;
	}
	return 0;
}
