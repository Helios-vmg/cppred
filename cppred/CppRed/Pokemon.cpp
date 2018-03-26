#include "stdafx.h"
#include "Pokemon.h"
#ifndef HAVE_PCH
#include <sstream>
#include <cmath>
#endif

namespace CppRed{

bool Party::add_pokemon(SpeciesId species, int level, std::uint16_t original_trainer_id, XorShift128 &rng){
	if (this->members.size() >= this->max_party_size)
		return false;
	this->members.emplace_back(species, level, original_trainer_id, rng);
	return true;
}

bool Party::add_pokemon(const Pokemon &pokemon){
	if (this->members.size() >= this->max_party_size)
		return false;
	this->members.push_back(pokemon);
	return true;
}

Pokemon &Party::get_last_added_pokemon(){
	if (!this->members.size())
		throw std::runtime_error("Internal error: Party::get_last_added_pokemon() called when no pokemon had been added.");
	return this->members.back();
}

void Party::heal(){
	for (auto &member : this->members)
		member.heal();
}

int Pokemon::get_stat(PokemonStats::StatId which, bool ignore_xp){
	auto &stat = this->computed_stats.get_stat(which);
	if (stat < 0){
		auto &pokemon = *pokemon_by_species_id[(int)this->species];
		int base_stat = pokemon.base_stats.get_stat(which);
		int bonus = 0;
		if (!ignore_xp){
			int stat_xp = this->stat_experience.get_stat(which);
			bonus = (int)ceil(sqrt(stat_xp));
		}
		int iv = this->get_iv(which);
		int ret = ((base_stat + iv) * 2 + bonus / 4) * this->level / 100;
		if (which == PokemonStats::StatId::Hp)
			ret += this->level + 10;
		else
			ret += 5;
		stat = ret;
	}
	return stat;
}

/*
 *
 * Explanation:
 *
 * individual value (bits): (most significant) FEDC BA98 7654 3210 (least significant)
 * attack  individual value = 7654
 * defense individual value = 3210
 * speed   individual value = FEDC
 * special individual value = BA98
 * HP      individual value = 40C8
 *
 * Example:
 *
 * IV = 48161 (0xBC21, 1011 1100 0010 0001)
 * attack  IV = 2  (0010)
 * defense IV = 1  (0001)
 * speed   IV = 12 (1100)
 * special IV = 11 (1011)
 * HP      IV = 6  (0110)
 *
 */
int Pokemon::get_iv(PokemonStats::StatId which) const{
	if (which == PokemonStats::StatId::Hp){
		int ret = 0;
		for (int i = 0; i < 4; i++){
			int index = (i ^ 1) * 4;
			int val = (this->individual_values >> index) & 0x01;
			ret <<= 1;
			ret |= val;
		}
		return ret;
	}
	int index = (int)which - 1;
	index ^= 1;
	return (this->individual_values >> index) & 0x0F;
}


Pokemon::Pokemon(SpeciesId species, int level, std::uint16_t original_trainer_id, XorShift128 &rng, const PokemonStats &input_stats){
	this->species = species;
	this->level = level;
	auto &pokemon = *pokemon_by_species_id[(int)species];
	fill(this->moves, MoveId::None);
	int i = 0;
	for (auto move : pokemon.get_initial_moves()){
		this->moves[i] = move;
		auto move_data = pokemon_moves_by_id[(int)move];
		this->pp[i] = move_data ? move_data->pp : 0;
		i++;
	}
	this->original_trainer_id = original_trainer_id;
	this->status = StatusCondition::Normal;
	rng.generate(this->individual_values);
	this->current_hp = this->get_stat(PokemonStats::StatId::Hp, true);
	this->experience = this->calculate_min_xp_to_reach_level(this->species, this->level);
	if (!input_stats.null())
		this->computed_stats = input_stats;
	else
		this->computed_stats.set_all(-1);
}

struct GrowthPolynomial{
	int a;
	int b;
	int c;
	int d;
	int e;
	int compute(int x) const{
		return x * x * x * a / b + x * x * c + x * d + e;
	}
};

static const GrowthPolynomial polynomials[] = {
	{ 1, 1,   0,   0,    0 },
	{ 3, 4,  10,   0,  -30 },
	{ 3, 4,  20,   0,  -70 },
	{ 6, 5, -15, 100, -140 },
	{ 4, 5,   0,   0,    0 },
	{ 5, 4,   0,   0,    0 },
};

static void check_rate(const BasePokemonInfo &data){
	auto growth_rate = data.growth_rate;
	if (growth_rate < 0 || growth_rate >= array_length(polynomials)){
		std::stringstream stream;
		stream << "Internal error: species " << data.internal_name << " has invalid growth rate of " << (int)growth_rate;
		throw std::runtime_error(stream.str());
	}
}

int Pokemon::calculate_min_xp_to_reach_level(SpeciesId species, int level){
	auto &data = *pokemon_by_species_id[(int)species];
	auto growth_rate = data.growth_rate;
	check_rate(data);
	return polynomials[growth_rate].compute(level);
}

int Pokemon::calculate_level_at_xp(SpeciesId species, int xp){
	auto &data = *pokemon_by_species_id[(int)species];
	auto growth_rate = data.growth_rate;
	check_rate(data);
	auto &poly = polynomials[growth_rate];
	int lo = 1,
		hi = 100;
	if (xp <= poly.compute(lo))
		return lo;
	if (xp >= poly.compute(hi))
		return hi;
	hi++;
	int d = hi - lo;
	while (d > 1){
		auto pivot = lo + d / 2;
		auto y = poly.compute(pivot);
		if (xp > y)
			lo = pivot;
		else if (xp < y)
			hi = pivot;
		else
			return pivot;
		d = hi - lo;
	}
	return lo;
}

void Pokemon::heal(){
	this->current_hp = this->get_stat(PokemonStats::StatId::Hp);
	this->status = StatusCondition::Normal;
}

}
