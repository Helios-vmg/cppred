#include "Pokemon.h"

namespace CppRed{

bool Party::add_pokemon(SpeciesId species, int level, std::uint16_t original_trainer_id, XorShift128 &rng){
	if (this->members.size() >= this->max_party_size)
		return false;
	this->members.emplace_back(species, level, original_trainer_id, rng);
	return true;
}

int Pokemon::get_stat(PokemonStats::StatId which, bool ignore_xp) const{
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
	return ret;
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
 */
int Pokemon::get_iv(PokemonStats::StatId which) const{
	if (which == PokemonStats::StatId::Hp){
		int ret = 0;
		for (int i = 0; i < 4; i++){
			int index = i ^= 1;
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


Pokemon::Pokemon(SpeciesId species, int level, std::uint16_t original_trainer_id, XorShift128 &rng){
	this->species = species;
	this->level = level;
	auto &pokemon = *pokemon_by_species_id[(int)species];
	fill(this->moves, MoveId::None);
	int i = 0;
	for (auto move : pokemon.get_initial_moves())
		this->moves[i++] = move;
	this->original_trainer_id = original_trainer_id;
	this->status = StatusCondition::Normal;
	rng.generate(this->individual_values);
	this->current_hp = this->get_stat(PokemonStats::StatId::Hp, true);

}

}
