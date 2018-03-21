#pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>

class TypeData{
	unsigned id;
	std::string name;
	std::string display_name;
public:
	TypeData(const std::vector<std::string> &columns);
	const std::string &get_name() const{
		return this->name;
	}
	unsigned get_id() const{
		return this->id;
	}
	const std::string &get_display_name() const{
		return this->display_name;
	}
};

class TypeStore{
	std::map<std::string, std::shared_ptr<TypeData>> types_by_name;
	std::map<unsigned, std::shared_ptr<TypeData>> types_by_id;
	std::map<std::string, unsigned> normalized_strings;
public:
	TypeStore(const char *path);
	std::shared_ptr<TypeData> get_type(const std::string &name) const;
	void generate_static_data_declarations(std::ostream &header) const;
	void generate_static_data_definitions(std::ostream &source) const;
};

class AdditionalEffectData{
	unsigned id;
	std::string name;
public:
	AdditionalEffectData(const std::vector<std::string> &columns);
	unsigned get_id() const{
		return this->id;
	}
	const std::string &get_name() const{
		return this->name;
	}
};

class AdditionalEffectStore{
	std::map<std::string, std::shared_ptr<AdditionalEffectData>> effects_by_name;
	std::map<unsigned, std::shared_ptr<AdditionalEffectData>> effects_by_id;
public:
	AdditionalEffectStore(const char *path);
	std::shared_ptr<AdditionalEffectData> get_effect(const std::string &name) const;
	void generate_static_data_declarations(std::ostream &header) const;
	//void generate_static_data_definitions(std::ostream &source) const;
};

class MoveData{
	unsigned id;
	std::string name;
	unsigned field_move_index;
	std::string display_name;
	std::shared_ptr<AdditionalEffectData> additional_effect;
	unsigned power;
	std::shared_ptr<TypeData> type;
	unsigned accuracy;
	unsigned pp;
public:
	MoveData(const std::vector<std::string> &columns, const TypeStore &types, const AdditionalEffectStore &);
	unsigned get_id() const{
		return this->id;
	}
	const std::string &get_name() const{
		return this->name;
	}
	const std::string &get_display_name() const{
		return this->display_name;
	}
	void output(std::ostream &) const;
};

class MoveStore{
	std::map<std::string, std::shared_ptr<MoveData>> moves_by_name;
	std::map<unsigned, std::shared_ptr<MoveData>> moves_by_id;
	std::vector<std::shared_ptr<MoveData>> moves_serialized;
public:
	MoveStore(const char *path, const TypeStore &types, const AdditionalEffectStore &);
	std::shared_ptr<MoveData> get_move(const std::string &name) const;
	void generate_static_data_declarations(std::ostream &header) const;
	void generate_static_data_definitions(std::ostream &source) const;
};

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
	std::shared_ptr<MoveData> move;

	LearnedMove(const std::vector<std::string> &columns, const MoveStore &moves);
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
	std::shared_ptr<TypeData> type[2];
	unsigned catch_rate;
	unsigned base_xp_yield;
	std::vector<std::shared_ptr<MoveData>> initial_attacks;
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
	std::string pokedex_entry = "ExclamationText";
	std::string brief;
	unsigned height_feet = 0,
		height_inches = 0,
		weight_tenths_of_pounds = 0;
	std::vector<EvolutionTrigger> evolution_triggers;
	std::vector<LearnedMove> learned_moves;

	SpeciesData(const std::vector<std::string> &columns, const MoveStore &moves, const TypeStore &types);

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
	TypeStore types;
	AdditionalEffectStore effects;
	MoveStore moves;
	std::vector<SpeciesData> species;
	std::map<std::string, unsigned> map;

	unsigned count_pokedex_species() const;
public:
	PokemonData();
	void generate_secondary_enums(const char *filename) const;
	void generate_enums(const char *filename) const;
	void generate_static_data_declarations(const char *filename) const;
	void generate_static_data_definitions(const char *filename, const char *header_name) const;
	const std::vector<SpeciesData> &get_species() const{
		return this->species;
	}
	const std::map<std::string, unsigned> &get_species_map();
	unsigned get_species_id(const std::string &name);
};
