#include "PokemonData.h"
#include "../common/csv_parser.h"
#include "utility.h"
#include <iomanip>
#include <algorithm>

extern const char * const pokemon_data_file = "input/pokemon_data.csv";
extern const char * const evolutions_file = "input/evolutions.csv";
extern const char * const pokemon_moves_file = "input/pokemon_moves.csv";
extern const char * const moves_file = "input/moves.csv";
extern const char * const pokemon_types_file = "input/pokemon_types.csv";
extern const char * const effects_file = "input/move_effects.csv";

PokemonData::PokemonData():
		types(pokemon_types_file),
		effects(effects_file),
		moves(moves_file, this->types, this->effects){
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
			"pokedex_entry",    // 27
			"description",		// 28
			"height_feet",		// 29
			"height_inches",	// 30
			"weight_pounds",	// 31
		};

		CsvParser csv(pokemon_data_file);
		unsigned rows = csv.row_count();
		for (unsigned i = 0; i < rows; i++)
			this->species.emplace_back(csv.get_ordered_row(i, data_order), this->moves, this->types);
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
			LearnedMove move(csv.get_ordered_row(i, moves_order), this->moves);
			species[move.species]->learned_moves.push_back(move);
		}
	}

	std::sort(this->species.begin(), this->species.end());
}

static unsigned parse_decimal_float(const std::string &s){
	if (s.size() < 2)
		return to_unsigned(s);
	int count = 0;
	for (auto c : s)
		count += c == '.';
	if (count > 1 || count && s[s.size() - 2] != '.')
		throw std::runtime_error("Invalid fraction: " + s + ". Must be in format \"12345678.9\", \".9\", or \"12345678\".");
	unsigned ret = 0;
	for (auto c : s){
		if (c == '.')
			continue;
		ret *= 10;
		ret += c - '0';
	}
	return ret;
}

SpeciesData::SpeciesData(const std::vector<std::string> &columns, const MoveStore &moves, const TypeStore &types){
	this->species_id = to_unsigned(columns[0]);
	this->pokedex_id = to_unsigned_default(columns[1]);
	this->name = columns[2];
	this->base_hp = to_unsigned_default(columns[3]);
	this->base_attack = to_unsigned_default(columns[4]);
	this->base_defense = to_unsigned_default(columns[5]);
	this->base_speed = to_unsigned_default(columns[6]);
	this->base_special = to_unsigned_default(columns[7]);
	try{
		for (int i = 0; i < 2; i++){
			auto &s = columns[8 + i];
			this->type[i] = types.get_type(!s.size() ? "Normal" : s);
		}
		this->catch_rate = to_unsigned_default(columns[10]);
		this->base_xp_yield = to_unsigned_default(columns[11]);
		for (int i = 0; i < 4; i++){
			auto &s = columns[12 + i];
			if (!s.size())
				continue;
			this->initial_attacks.push_back(moves.get_move(s));
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
		if (this->allocated){
			this->pokedex_entry = columns[27];
			this->brief = columns[28];
			this->height_feet = to_unsigned(columns[29]);
			this->height_inches = to_unsigned(columns[30]);
			this->weight_tenths_of_pounds = parse_decimal_float(columns[31]);
		}
	}catch (std::exception &e){
		throw std::runtime_error("Error while processing Pokemon data for " + this->name + ": " + e.what());
	}
}

EvolutionTrigger::EvolutionTrigger(const std::vector<std::string> &columns){
	this->species = columns[0];
	this->type = columns[1];
	this->minimum_level = to_unsigned(columns[2]);
	this->next_form = columns[3];
	this->item = columns[4];
}

LearnedMove::LearnedMove(const std::vector<std::string> &columns, const MoveStore &moves){
	this->species = columns[0];
	this->level = to_unsigned(columns[1]);
	this->move = moves.get_move(columns[2]);
}

void PokemonData::generate_secondary_enums(const char *filename) const{
	std::ofstream file(filename);
	file << generated_file_warning <<
		"\n"
		"#pragma once\n"
		"\n";
	
	this->types.generate_static_data_declarations(file);
	this->effects.generate_static_data_declarations(file);
	this->moves.generate_static_data_declarations(file);
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
		"static const size_t pokemon_by_species_id_size = " << this->species.size() << ";\n"
		"extern const BasePokemonInfo * const pokemon_by_pokedex_id[" << this->count_pokedex_species() << "];\n"
		"static const size_t pokemon_by_pokedex_id_size = " << this->count_pokedex_species() << ";\n";
}

void PokemonData::generate_static_data_definitions(const char *filename, const char *header_name) const{
	std::ofstream file(filename);

	file << generated_file_warning <<
		"\n"
		"#include \"" << header_name << "\"\n"
		"\n";
	
	this->types.generate_static_data_definitions(file);
	this->moves.generate_static_data_definitions(file);

	for (auto &species : this->species){
		file <<
			"const PokemonInfo<" << species.initial_attacks.size() << ", " << species.evolution_triggers.size() << ", " << species.learned_moves.size() << "> pokemoninfo_" << species.name << " = {\n"
			"    PokedexId::" << (species.pokedex_id ? species.name : "None") << ",\n"
			"    SpeciesId::" << species.name << ",\n"
			"    \"" << species.name << "\",\n"
			"    " << bool_to_string(species.allocated) << ",\n"
			"    " << species.starter_index << ",\n"
			"    " << species.base_hp << ",\n"
			"    " << species.base_attack << ",\n"
			"    " << species.base_defense << ",\n"
			"    " << species.base_speed << ",\n"
			"    " << species.base_special << ",\n"
			"    {\n"
			"        PokemonTypeId::" << species.type[0]->get_name() << ",\n"
			"        PokemonTypeId::" << species.type[1]->get_name() << ",\n"
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
			"    TextResourceId::" << species.pokedex_entry << ",\n";
		if (species.brief.size())
			file <<
			"    \"" << species.brief << "\",\n";
		else
			file <<
			"    nullptr,\n";
		file <<
			"    " << species.height_feet << ",\n"
			"    " << species.height_inches<< ",\n"
			"    " << species.weight_tenths_of_pounds << ",\n"
			"    {\n"
			;

		for (auto &attack : species.initial_attacks)
			file << "        MoveId::" << attack->get_name() << ",\n";

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
			file << "        LEARN(" << move.level << ", " << move.move->get_name() << "),\n";
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

MoveStore::MoveStore(const char *path, const TypeStore &types, const AdditionalEffectStore &effects){
	static const std::vector<std::string> data_order = {
		"id",
		"name",
		"field_move_index",
		"display_name",
		"additional_effect",
		"power",
		"type",
		"accuracy",
		"pp",
	};

	CsvParser csv(path);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, data_order);
		auto move = std::make_shared<MoveData>(row, types, effects);
		this->moves_by_name[move->get_name()] = move;
		this->moves_by_id[move->get_id()] = move;
	}
	this->moves_serialized.resize(this->moves_by_id.rbegin()->second->get_id() + 1);
	for (auto &kv : this->moves_by_id)
		if (kv.second->get_display_name().size())
			this->moves_serialized[kv.first] = kv.second;
}

std::shared_ptr<MoveData> MoveStore::get_move(const std::string &name) const{
	auto it = this->moves_by_name.find(name);
	if (it == this->moves_by_name.end())
		throw std::runtime_error("Move not found: " + name);
	return it->second;
}

MoveData::MoveData(const std::vector<std::string> &columns, const TypeStore &types, const AdditionalEffectStore &effects){
	this->id                = to_unsigned(columns[0]);
	this->name              =             columns[1] ;
	this->field_move_index  = to_unsigned(columns[2]);
	this->display_name      =             columns[3] ;
	this->power             = to_unsigned(columns[5]);
	this->accuracy          = to_unsigned(columns[7]);
	this->pp                = to_unsigned(columns[8]);

	try{
		this->additional_effect = effects.get_effect(columns[4]);
		this->type = types.get_type(columns[6]);
	}catch (std::exception &e){
		throw std::runtime_error("Error while processing move data for " + this->name + ": " + e.what());
	}
}

TypeData::TypeData(const std::vector<std::string> &columns){
	this->id           = to_unsigned(columns[0]);
	this->name         =             columns[1] ;
	this->display_name =             columns[2] ;
}

TypeStore::TypeStore(const char *path){
	static const std::vector<std::string> data_order = {
		"id",
		"name",
		"display_name",
	};

	CsvParser csv(path);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, data_order);
		auto move = std::make_shared<TypeData>(row);
		this->types_by_name[move->get_name()] = move;
		this->types_by_id[move->get_id()] = move;
		unsigned id = this->normalized_strings.size();
		this->normalized_strings[move->get_display_name()] = id;
	}
}

AdditionalEffectStore::AdditionalEffectStore(const char *path){
	static const std::vector<std::string> data_order = {
		"id",
		"name",
	};

	CsvParser csv(path);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, data_order);
		auto effect = std::make_shared<AdditionalEffectData>(row);
		this->effects_by_name[effect->get_name()] = effect;
		this->effects_by_id[effect->get_id()] = effect;
	}
}

AdditionalEffectData::AdditionalEffectData(const std::vector<std::string> &columns){
	this->id = to_unsigned(columns[0]);
	this->name = columns[1];
}

std::shared_ptr<AdditionalEffectData> AdditionalEffectStore::get_effect(const std::string &name) const{
	auto it = this->effects_by_name.find(name);
	if (it == this->effects_by_name.end())
		throw std::runtime_error("Effect not found: " + name);
	return it->second;
}

std::shared_ptr<TypeData> TypeStore::get_type(const std::string &name) const{
	auto it = this->types_by_name.find(name);
	if (it == this->types_by_name.end())
		throw std::runtime_error("Type not found: " + name);
	return it->second;
}

void TypeStore::generate_static_data_declarations(std::ostream &header) const{
	header << "enum class PokemonTypeId{\n";
	for (auto &kv : this->types_by_id)
		header << "    " << kv.second->get_name() << " = " << kv.second->get_id() << ",\n";
	header <<
		"};\n"
		"\n"
		"extern const char * const pokemon_type_strings[" << this->types_by_id.size() << "];\n"
		"\n";
}

void TypeStore::generate_static_data_definitions(std::ostream &source) const{
	source << "const char * const pokemon_type_strings2[] = {\n";
	{
		std::map<unsigned, std::string> temp_inverse;
		for (auto &kv : this->normalized_strings)
			temp_inverse[kv.second] = kv.first;
		for (auto &kv : temp_inverse)
			source << "    \"" << kv.second << "\",\n";
	}
	source <<
		"};\n"
		"\n"
		"extern const char * const pokemon_type_strings[" << this->types_by_id.size() << "] = {\n";
	for (auto &kv : this->types_by_id)
		source << "    pokemon_type_strings2[" << this->normalized_strings.find(kv.second->get_display_name())->second << "],\n";
	source << "};\n";
}

void AdditionalEffectStore::generate_static_data_declarations(std::ostream &header) const{
	header << "enum class MoveAdditionalEffect{\n";
	for (auto &kv : this->effects_by_id)
		header << "    " << kv.second->get_name() << " = " << kv.second->get_id() << ",\n";
	header <<
		"};\n"
		"\n";
}

void MoveStore::generate_static_data_declarations(std::ostream &header) const{
	header << "enum class MoveId{\n";
	for (auto &kv : this->moves_by_id)
		header << "    " << kv.second->get_name() << " = " << kv.second->get_id() << ",\n";

	header <<
		"};\n"
		"\n"
		"struct MoveData;\n"
		"\n"
		"extern const MoveData * const pokemon_moves_by_id[" << this->moves_serialized.size() << "];\n"
		"static const size_t pokemon_moves_by_id_size = " << this->moves_serialized.size() << ";\n";
}

void MoveStore::generate_static_data_definitions(std::ostream &source) const{
	for (auto &kv : this->moves_by_name){
		if (!kv.second->get_display_name().size())
			continue;
		kv.second->output(source);
		source << '\n';
	}
	source << "\n"
		"extern const MoveData * const pokemon_moves_by_id[" << this->moves_serialized.size() << "] = {\n";
	for (auto &p : this->moves_serialized){
		if (p)
			source << "    &move_" << p->get_name();
		else
			source << "    nullptr";
		source << ",\n";
	}
	source << "};\n";
}

void MoveData::output(std::ostream &stream) const{
	stream << "static const MoveData move_" << this->name << "("
		"MoveId::" << this->name << ", "
		"\"" << this->name << "\", "
		<< this->field_move_index << ", "
		"\"" << this->display_name << "\", "
		"MoveAdditionalEffect::" << this->additional_effect->get_name() << ", "
		<< this->power << ", "
		"PokemonTypeId::" << this->type->get_name() << ", "
		<< this->accuracy << ", "
		<< this->pp << ");";
}
