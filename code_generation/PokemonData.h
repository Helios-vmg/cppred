#pragma once
#include <vector>

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
	std::string overworld_sprite;
	unsigned cry_base;
	unsigned cry_pitch;
	unsigned cry_length;
	bool allocated;
	int starter_index;
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
	std::vector<SpeciesData> species;

	unsigned count_pokedex_species() const;
public:
	PokemonData();
	void generate_enums(const char *filename) const;
	void generate_static_data_declarations(const char *filename) const;
	void generate_static_data_definitions(const char *filename, const char *header_name) const;
};
