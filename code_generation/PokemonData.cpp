#include "PokemonData.h"
#include "../common/csv_parser.h"
#include "utility.h"
#include <iomanip>
#include <algorithm>

extern const char * const pokemon_data_file = "input/pokemon_data.csv";
extern const char * const evolutions_file = "input/evolutions.csv";
extern const char * const pokemon_moves_file = "input/pokemon_moves.csv";

PokemonData::PokemonData(){
	{
		static const std::vector<std::string> data_order = {
			"species_id",       // 0
			"pokedex_id",       // 1
			"name",             // 2
			"base_hp",          // 3
			"base_attack",      // 4
			"base_defense",     // 5
			"base_speed",       // 6
			"base_special",     // 7
			"type1",            // 8
			"type2",            // 9
			"catch_rate",       // 10
			"base_xp_yield",    // 11
			"initial_attack1",  // 12
			"initial_attack2",  // 13
			"initial_attack3",  // 14
			"initial_attack4",  // 15
			"growth_rate",      // 16
			"tmlearn_bitmap",   // 17
			"display_name",     // 18
			"front_image",      // 19
			"back_image",       // 20
			"cry_base",         // 21
			"cry_pitch",        // 22
			"cry_length",       // 23
			"allocated",        // 24
			"overworld_sprite", // 25
			"starter_index",    // 26
		};

		CsvParser csv(pokemon_data_file);
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++)
			this->species.push_back(SpeciesData(csv.get_ordered_row(i, data_order)));
	}

	std::map<std::string, SpeciesData *> species;
	for (auto &s : this->species)
		species[s.name] = &s;

	{
		static const std::vector<std::string> evolutions_order = {
			"species",       // 0
			"type",          // 1
			"minimum_level", // 2
			"next_form",     // 3
			"item",          // 4
		};

		CsvParser csv(evolutions_file);
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++){
			EvolutionTrigger evolution(csv.get_ordered_row(i, evolutions_order));
			species[evolution.species]->evolution_triggers.push_back(evolution);
		}
	}
	{
		const std::vector<std::string> moves_order = {
			"species", // 0
			"level",   // 1
			"move",    // 2
		};

		CsvParser csv(pokemon_moves_file);
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++){
			LearnedMove move(csv.get_ordered_row(i, moves_order));
			species[move.species]->learned_moves.push_back(move);
		}
	}

	std::sort(this->species.begin(), this->species.end());
}

static const std::pair<std::string, std::string> charmap[] = {
	{ "\\\\",       "\\\\01" },
	{ "<POKE>",     "\\\\02" },
	{ "<pkmn>",     "\\\\03" },
	{ "<PLAYER>",   "\\\\04" },
	{ "<RIVAL>",    "\\\\05" },
	{ "<USER>",     "\\\\06" },
	{ "<TARGET>",   "\\\\07" },
	{ "<CURRENCY>", "\\\\08" },
	{ "'d",         "\\\\09" },
	{ "'l",         "\\\\0A" },
	{ "'s",         "\\\\0B" },
	{ "'t",         "\\\\0C" },
	{ "'v",         "\\\\0D" },
	{ "'r",         "\\\\0E" },
	{ "'m",         "\\\\0F" },
	{ "<FEMALE>",   "\\\\10" },
	{ "<MALE>",     "\\\\11" },
};

static std::string filter_text(const std::string &input){
	std::string ret;
	for (size_t i = 0; i < input.size(); ){
		auto c = input[i];
		if (c == '@'){
			i++;
			continue;
		}
		bool Continue = false;
		for (auto &p : charmap){
			if (p.first.size() > input.size() - i)
				continue;
			if (p.first != input.substr(i, p.first.size()))
				continue;
			ret += p.second;
			i += p.first.size();
			Continue = true;
			break;
		}
		if (Continue)
			continue;
		if (c == '<'){
			if (i + 2 > input.size())
				throw std::runtime_error("Syntax error: string can't contain '<': " + input);
			if (input[i + 1] != '$')
				throw std::runtime_error("Missed a case: " + input);
			if (i + 5 > input.size())
				throw std::runtime_error("Syntax error: Incomplete <$XX> sequence: " + input);
			auto a = input[i + 2];
			auto b = input[i + 3];
			if (!is_hex(a) || !is_hex(b) || input[i + 4] != '>')
				throw std::runtime_error("Syntax error: Invalid <$XX> sequence: " + input);
			if (a == '0' && b == '0')
				throw std::runtime_error("Internal error: Can't represent <$00>: " + input);
			ret += "\\\\x\\x";
			ret += (char)toupper(a);
			ret += (char)toupper(b);
			ret += "\" \"";
			i += 5;
			continue;
		}
		ret += c;
		i++;
	}
	return ret;
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
	this->display_name = filter_text(columns[18]);
	this->front_image = columns[19];
	this->back_image = columns[20];
	this->cry_base = hex_no_prefix_to_unsigned_default(columns[21]);
	this->cry_pitch = hex_no_prefix_to_unsigned_default(columns[22]);
	this->cry_length = hex_no_prefix_to_unsigned_default(columns[23]);

	if (this->front_image.size())
		this->front_image = "&" + this->front_image;
	else
		this->front_image = "nullptr";
	if (this->back_image.size())
		this->back_image = "&" + this->back_image;
	else
		this->back_image = "nullptr";

	this->allocated = to_bool(columns[24]);
	this->overworld_sprite = columns[25];
	this->starter_index = to_int(columns[26]);
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

	file << generated_file_warning <<
		"\n"
		"#pragma once\n"
		"\n"
		"enum class SpeciesId{\n";
	{
		std::vector<std::pair<unsigned, std::string>> temp;
		for (auto &species : this->species)
			temp.push_back({ species.species_id, species.name });
		std::sort(temp.begin(), temp.end());
		for (auto &species : temp)
			file << "    " << species.second << " = " << species.first << ",\n";
	}
	file <<
		"};\n"
		"\n"
		"enum class PokedexId{\n";
	int count = 0;
	for (auto &species : this->species){
		if (!species.pokedex_id && species.species_id)
			break;
		count += species.allocated;
		file << "    " << species.name << " = " << species.pokedex_id << ",\n";
	}
	file << "};\n"
		"static const int pokemon_species_count = " << count << ";\n";
}

void PokemonData::generate_static_data_declarations(const char *filename) const{
	std::ofstream file(filename);

	file << generated_file_warning <<
		"\n"
		"#pragma once\n"
		"\n";

	for (auto &species : this->species)
		file << "extern const PokemonInfo<" << species.initial_attacks.size() << ", " << species.evolution_triggers.size() << ", " << species.learned_moves.size() << "> pokemoninfo_" << species.name << ";\n";

	file <<
		"extern const BasePokemonInfo * const pokemon_by_species_id[" << this->species.size() << "];\n"
		"extern const BasePokemonInfo * const pokemon_by_pokedex_id[" << this->count_pokedex_species() << "];\n";
}

void PokemonData::generate_static_data_definitions(const char *filename, const char *header_name) const{
	std::ofstream file(filename);

	file << generated_file_warning <<
		"\n"
		"#include \"" << header_name << "\"\n"
		"\n";

	for (auto &species : this->species){
		file <<
			"const PokemonInfo<" << species.initial_attacks.size() << ", " << species.evolution_triggers.size() << ", " << species.learned_moves.size() << "> pokemoninfo_" << species.name << " = {\n"
			"    PokedexId::" << (species.pokedex_id ? species.name : "None") << ",\n"
			"    SpeciesId::" << species.name << ",\n"
			"    " << bool_to_string(species.allocated) << ",\n"
			"    " << species.starter_index << ",\n"
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
			"    PokemonOverworldSprite::" << species.overworld_sprite << ",\n"
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
			file << "    &pokemoninfo_" << species.second << ",\n";
		file <<
			"};\n"
			"\n";
	}
	{
		file << "const BasePokemonInfo * const pokemon_by_pokedex_id[" << this->count_pokedex_species() << "] = {\n";
		for (auto &species : this->species){
			if (!species.pokedex_id)
				continue;
			file << "    &pokemoninfo_" << species.name << ",\n";
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

const std::map<std::string, unsigned> &PokemonData::get_species_map(){
	if (!this->map.size()){
		for (auto &p : this->species)
			this->map[p.name] = p.species_id;
	}
	return this->map;
}

unsigned PokemonData::get_species_id(const std::string &name){
	auto &map = this->get_species_map();
	auto it = map.find(name);
	if (it == map.end())
		throw std::runtime_error("Error: Invalid SpeciesId: " + name);
	return it->second;
}
