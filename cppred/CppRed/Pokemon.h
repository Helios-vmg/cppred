#pragma once
#include "Data.h"
#include "Status.h"
#include "utility.h"
#include <cstdint>

struct PokemonStats{
	int hp;
	int attack;
	int defense;
	int speed;
	int special;
};

class CppRedPokemon{
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
	CppRedPokemon(SpeciesId species, int level, XorShift128 &);
	CppRedPokemon(const CppRedPokemon &);
	CppRedPokemon(CppRedPokemon &&);
	const CppRedPokemon &operator=(const CppRedPokemon &);
	const CppRedPokemon &operator=(CppRedPokemon &&);
};

class CppRedParty{
public:
	static const size_t max_party_size = 6;
private:
	std::vector<CppRedPokemon> members;
public:
	CppRedParty() = default;
};
