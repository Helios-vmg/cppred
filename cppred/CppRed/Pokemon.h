#pragma once
#include "Data.h"
#include "Status.h"
#include "utility.h"
#include <cstdint>

namespace CppRed{

struct PokemonStats{
	int hp;
	int attack;
	int defense;
	int speed;
	int special;
};

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
	byte_t dvs[2];
	byte_t pp[4];
public:
	//Generates a random pokemon with the given species and level.
	Pokemon(SpeciesId species, int level, XorShift128 &);
	Pokemon(const Pokemon &);
	Pokemon(Pokemon &&);
	const Pokemon &operator=(const Pokemon &);
	const Pokemon &operator=(Pokemon &&);
};

class Party{
public:
	static const size_t max_party_size = 6;
private:
	std::vector<Pokemon> members;
public:
	Party() = default;
};

}
