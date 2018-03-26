#pragma once
#include "common_types.h"
#include "../CodeGeneration/output/pokemon_moves.h"
#include "../CodeGeneration/output/pokemon_enums.h"
#include "../CodeGeneration/output/items.h"
#include "../CodeGeneration/output/text.h"
#include "GraphicsAsset.h"
#ifndef HAVE_PCH
#include <array>
#include <vector>
#endif

struct MoveData{
	MoveId id;
	const char *name;
	int field_move_index;
	const char *display_name;
	MoveAdditionalEffect additional_effect;
	int power;
	PokemonTypeId type;
	int accuracy;
	int pp;
	explicit MoveData(
		MoveId id,
		const char *name,
		int field_move_index,
		const char *display_name,
		MoveAdditionalEffect additional_effect,
		int power,
		PokemonTypeId type,
		int accuracy,
		int pp):
		id(id),
		name(name),
		field_move_index(field_move_index),
		display_name(display_name),
		additional_effect(additional_effect),
		power(power),
		type(type),
		accuracy(accuracy),
		pp(pp){}
};

enum class PokemonOverworldSprite{
	Mon       = 0, //Generic mon
	Ball      = 1, //Pokeball
	Helix     = 2, //Spiral shell
	Fairy     = 3, //Clefairy-like
	Bird      = 4, //Bird
	Water     = 5, //Seal
	Bug       = 6, //Bug
	Grass     = 7, //Flower with eyes
	Snake     = 8, //Snake
	Quadruped = 9, //Bovine
	Invalid   = 10,
	Invalid11 = 11,
	Invalid12 = 12,
	Invalid13 = 13,
	Invalid14 = 14,
	Invalid15 = 15,
	Count,
};

struct PokemonCryData{
	byte_t base_cry;
	byte_t pitch;
	byte_t length;
};

enum class EvolutionTriggerType{
	None,
	Level,
	Item,
	Trade,
};

struct EvolutionTrigger{
	EvolutionTriggerType type;
	byte_t minimum_level;
	SpeciesId next_form;
	ItemId trigger_item;
};

struct LearnedMove{
	byte_t level;
	MoveId move;
};

struct PokemonStats{
	int hp;
	int attack;
	int defense;
	int speed;
	int special;

	PokemonStats():
		hp(0),
		attack(0),
		defense(0),
		speed(0),
		special(0){}
	PokemonStats(int hp, int attack, int defense, int speed, int special):
		hp(hp),
		attack(attack),
		defense(defense),
		speed(speed),
		special(special){}

	enum class StatId{
		Hp = 0,
		Attack,
		Defense,
		Speed,
		Special,
	};
	int &get_stat(StatId id){
		switch (id){
			case StatId::Hp:
				return this->hp;
			case StatId::Attack:
				return this->attack;
			case StatId::Defense:
				return this->defense;
			case StatId::Speed:
				return this->speed;
			case StatId::Special:
				return this->special;
		}
		return this->special;
	}
	int get_stat(StatId id) const{
		switch (id){
			case StatId::Hp:
				return this->hp;
			case StatId::Attack:
				return this->attack;
			case StatId::Defense:
				return this->defense;
			case StatId::Speed:
				return this->speed;
			case StatId::Special:
				return this->special;
		}
		return this->special;
	}
	bool null() const{
		return !this->hp && !this->attack && !this->defense && !this->defense && !this->speed && !this->special;
	}
	void set_all(int value){
		this->hp = 
			this->attack = 
			this->defense = 
			this->speed = 
			this->special = value;
	}
};

struct BasePokemonInfo{
	PokedexId pokedex_id;
	SpeciesId species_id;
	const char *internal_name;
	bool allocated;
	std::int8_t starter_index;
	PokemonStats base_stats;
	std::array<PokemonTypeId, 2> type;
	byte_t catch_rate;
	byte_t base_xp_yield;
	byte_t growth_rate;
	std::array<byte_t, 7> learnable_tms_bitmap;
	const char * const display_name;
	const GraphicsAsset * const front;
	const GraphicsAsset * const back;
	PokemonOverworldSprite overworld_sprite;
	PokemonCryData cry_data;
	TextResourceId pokedex_entry;
	const char * const brief;
	byte_t height_feet;
	byte_t height_inches;
	int weight_tenths_of_pounds;

	BasePokemonInfo(
		PokedexId pokedex_id,
		SpeciesId species_id,
		const char *internal_name,
		bool allocated,
		std::int8_t starter_index,
		byte_t base_hp,
		byte_t base_attack,
		byte_t base_defense,
		byte_t base_speed,
		byte_t base_special,
		std::array<PokemonTypeId, 2> &&type,
		byte_t catch_rate,
		byte_t base_xp_yield,
		byte_t growth_rate,
		std::array<byte_t, 7> &&learnable_tms_bitmap,
		const char *display_name,
		const GraphicsAsset * const front,
		const GraphicsAsset * const back,
		PokemonOverworldSprite overworld_sprite,
		PokemonCryData &&cry_data,
		TextResourceId pokedex_entry,
		const char * const brief,
		byte_t height_feet,
		byte_t height_inches,
		int weight_tenths_of_pounds
	):
		pokedex_id(pokedex_id),
		species_id(species_id),
		internal_name(internal_name),
		allocated(allocated),
		starter_index(starter_index),
		base_stats{base_hp, base_attack, base_defense, base_speed, base_special},
		type(std::move(type)),
		catch_rate(catch_rate),
		base_xp_yield(base_xp_yield),
		growth_rate(growth_rate),
		learnable_tms_bitmap(std::move(learnable_tms_bitmap)),
		display_name(display_name),
		front(front),
		back(back),
		overworld_sprite(overworld_sprite),
		cry_data(cry_data),
		pokedex_entry(pokedex_entry),
		brief(brief),
		height_feet(height_feet),
		height_inches(height_inches),
		weight_tenths_of_pounds(weight_tenths_of_pounds)
	{}
	BasePokemonInfo(const BasePokemonInfo &) = delete;
	BasePokemonInfo(BasePokemonInfo &&) = delete;
	void operator=(const BasePokemonInfo &) = delete;
	void operator=(BasePokemonInfo &&) = delete;
	virtual std::vector<MoveId> get_initial_moves() const = 0;
	virtual std::vector<EvolutionTrigger> get_evolution_triggers() const = 0;
	virtual std::vector<LearnedMove> get_learned_moves() const = 0;
};

template <size_t N1, size_t N2, size_t N3>
struct PokemonInfo : public BasePokemonInfo{
private:
	template <typename T, size_t N>
	static std::vector<T> to_vector(const std::array<T, N> &a){
		std::vector<T> ret;
		ret.reserve(N);
		for (auto &i : a)
			ret.push_back(i);
		return ret;
	}
public:
	std::array<MoveId, N1> initial_attacks;
	std::array<EvolutionTrigger, N2> evolution_triggers;
	std::array<LearnedMove, N3> learned_moves;

	PokemonInfo(
		PokedexId pokedex_id,
		SpeciesId species_id,
		const char *internal_name,
		bool allocated,
		std::int8_t starter_index,
		byte_t base_hp,
		byte_t base_attack,
		byte_t base_defense,
		byte_t base_speed,
		byte_t base_special,
		std::array<PokemonTypeId, 2> &&type,
		byte_t catch_rate,
		byte_t base_xp_yield,
		byte_t growth_rate,
		std::array<byte_t, 7> &&learnable_tms_bitmap,
		const char *display_name,
		const GraphicsAsset * const front,
		const GraphicsAsset * const back,
		PokemonOverworldSprite overworld_sprite,
		PokemonCryData &&cry_data,
		TextResourceId pokedex_entry,
		const char * const brief,
		byte_t height_feet,
		byte_t height_inches,
		int weight_tenths_of_pounds,
		std::array<MoveId, N1> &&initial_attacks,
		std::array<EvolutionTrigger, N2> &&evolution_triggers,
		std::array<LearnedMove, N3> &&learned_moves
	):
	BasePokemonInfo(
		pokedex_id,
		species_id,
		internal_name,
		allocated,
		starter_index,
		base_hp,
		base_attack,
		base_defense,
		base_speed,
		base_special,
		std::move(type),
		catch_rate,
		base_xp_yield,
		growth_rate,
		std::move(learnable_tms_bitmap),
		display_name,
		front,
		back,
		overworld_sprite,
		std::move(cry_data),
		pokedex_entry,
		brief,
		height_feet,
		height_inches,
		weight_tenths_of_pounds
	),
		initial_attacks(std::move(initial_attacks)),
		evolution_triggers(std::move(evolution_triggers)),
		learned_moves(std::move(learned_moves))
	{}

	std::vector<MoveId> get_initial_moves() const override{
		return to_vector(this->initial_attacks);
	}
	std::vector<EvolutionTrigger> get_evolution_triggers() const override{
		return to_vector(this->evolution_triggers);
	}
	std::vector<LearnedMove> get_learned_moves() const override{
		return to_vector(this->learned_moves);
	}
};
