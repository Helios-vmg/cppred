#pragma once
#include "Data.h"
#include "Status.h"
#include "utility.h"
#include <cstdint>

namespace CppRed{

class Pokemon{
	SpeciesId species;
	std::string nickname;
	int current_hp;
	int level;
	StatusCondition status;
	MoveId moves[4];
	std::uint16_t original_trainer_id;
	int experience;
	PokemonStats stat_experience;
	PokemonStats computed_stats;
	std::uint16_t individual_values;
	int pp[4];
	
	int get_iv(PokemonStats::StatId) const;
public:
	Pokemon():
		species(SpeciesId::None),
		current_hp(0),
		level(0),
		status(StatusCondition::Normal),
		original_trainer_id(0),
		experience(0),
		individual_values(0){}
	//Generates a random pokemon with the given species and level.
	Pokemon(SpeciesId species, int level, std::uint16_t original_trainer_id, XorShift128 &, const PokemonStats &input_stats = PokemonStats());
	Pokemon(const Pokemon &) = default;
	Pokemon(Pokemon &&) = default;
	int get_stat(PokemonStats::StatId, bool ignore_xp = false);
	static int calculate_min_xp_to_reach_level(SpeciesId species, int level);
	static int calculate_level_at_xp(SpeciesId species, int xp);
	bool null() const{
		return this->species == SpeciesId::None;
	}
	DEFINE_GETTER(species)
	DEFINE_GETTER_SETTER(nickname)
};

class Party{
public:
	static const size_t max_party_size = 6;
private:
	std::vector<Pokemon> members;
public:
	Party() = default;
	bool add_pokemon(SpeciesId, int level, std::uint16_t original_trainer_id, XorShift128 &);
	bool add_pokemon(const Pokemon &);
	Pokemon &get_last_added_pokemon();
};

}
