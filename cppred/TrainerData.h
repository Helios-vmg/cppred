#pragma once
#include "GraphicsAsset.h"
#include "utility.h"
#ifndef HAVE_PCH
#include <array>
#include <memory>
#include <string>
#include <vector>
#endif

enum class SpeciesId;
enum class TrainerId;

struct TrainerPartyMember{
	SpeciesId species;
	int level;
};

class PartialTrainerClass{
protected:
	TrainerId id;
	std::string name;
	std::string display_name;
	const GraphicsAsset *graphics;
	int base_reward;
	int ai_move_choice_modifiers[4];
	std::string ai_class;
	int ai_parameter;
public:
	PartialTrainerClass() = default;
	PartialTrainerClass(BufferReader &);
	PartialTrainerClass(const PartialTrainerClass &) = default;
	PartialTrainerClass(const PartialTrainerClass &&other){
		*this = std::move(other);
	}
	PartialTrainerClass &operator=(const PartialTrainerClass &) = default;
	const PartialTrainerClass &operator=(PartialTrainerClass &&);
	
	DEFINE_GETTER(id)
	DEFINE_GETTER(name)
	DEFINE_GETTER(display_name)
	DEFINE_GETTER(graphics)
	DEFINE_GETTER(base_reward)
	DEFINE_GETTER(ai_move_choice_modifiers)
	DEFINE_GETTER(ai_class)
	DEFINE_GETTER(ai_parameter)
};

class FullTrainerClass : public PartialTrainerClass{
	std::vector<TrainerPartyMember> party;
public:
	FullTrainerClass(const PartialTrainerClass &ptc, const std::vector<TrainerPartyMember> &party):
		PartialTrainerClass(ptc),
		party(party){}
	FullTrainerClass(FullTrainerClass &&other):
			PartialTrainerClass(){
		this->party = std::move(other.party);
	}
	const FullTrainerClass &operator=(FullTrainerClass &&);
	DEFINE_GETTER(party);
};

class TrainerClassData : public PartialTrainerClass{
	std::vector<std::vector<TrainerPartyMember>> parties;
public:
	TrainerClassData(BufferReader &);
	const std::vector<TrainerPartyMember> &get_party(size_t) const;
	FullTrainerClass get_trainer(size_t) const;
};

class TrainerClassesStore{
	std::vector<std::shared_ptr<TrainerClassData>> trainers_by_id;
	std::vector<std::shared_ptr<TrainerClassData>> trainers_by_name;
public:
	TrainerClassesStore(BufferReader &);
	std::shared_ptr<TrainerClassData> get_trainer_class_by_id(TrainerId) const;
	std::shared_ptr<TrainerClassData> get_trainer_by_name(const char *) const;
	FullTrainerClass get_trainer_class_by_id(TrainerId id, size_t index) const{
		return this->get_trainer_class_by_id(id)->get_trainer(index);
	}
	FullTrainerClass get_trainer_by_name(const char *name, size_t index) const{
		return this->get_trainer_by_name(name)->get_trainer(index);
	}
};
