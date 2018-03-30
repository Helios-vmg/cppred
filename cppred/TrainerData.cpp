#include "stdafx.h"
#include "TrainerData.h"
#include "../CodeGeneration/output/graphics_public.h"

static bool compare_by_id(const TrainerClassData &a, const TrainerClassData &b){
	return a.get_id() < b.get_id();
}

static bool compare_by_name(const TrainerClassData &a, const TrainerClassData &b){
	return a.get_name() < b.get_name();
}

TrainerClassesStore::TrainerClassesStore(BufferReader &buffer){
	auto trainers = buffer.read_varint();
	this->trainers_by_id.reserve(trainers);
	this->trainers_by_name.reserve(trainers);
	for (auto i = trainers; i--;){
		this->trainers_by_id.emplace_back(new TrainerClassData(buffer));
		this->trainers_by_name.emplace_back(this->trainers_by_id.back());
	}
	std::sort(this->trainers_by_id.begin(), this->trainers_by_id.end(), [](const auto &a, const auto &b){ return (int)a->get_id() < (int)b->get_id(); });
	std::sort(this->trainers_by_name.begin(), this->trainers_by_name.end(), [](const auto &a, const auto &b){ return a->get_name() < b->get_name(); });
}

std::shared_ptr<TrainerClassData> TrainerClassesStore::get_trainer_class_by_id(TrainerId id_) const{
	auto id = (int)id_;
	auto b = this->trainers_by_id.begin();
	auto e = this->trainers_by_id.end();
	auto it = find_first_true(b, e, [id](const auto &i){ return (int)i->get_id() >= id; });
	if (it == e || (*it)->get_id() != id_){
		std::stringstream stream;
		stream << "TrainerClassesStore::get_trainer_by_id(): Internal error. Invalid trainer class id " << id;
		throw std::runtime_error(stream.str());
	}
	return *it;
}

std::shared_ptr<TrainerClassData> TrainerClassesStore::get_trainer_by_name(const char *name) const{
	auto b = this->trainers_by_name.begin();
	auto e = this->trainers_by_name.end();
	auto it = find_first_true(b, e, [name](const auto &i){ return i->get_name() >= name; });
	if (it == e || (*it)->get_name() != name)
		throw std::runtime_error((std::string)"TrainerClassesStore::get_trainer_by_name(): Internal error. Invalid trainer class name " + name);
	return *it;
}

PartialTrainerClass::PartialTrainerClass(BufferReader &buffer){
	this->id = (TrainerId)buffer.read_varint();
	this->name = buffer.read_string();
	this->display_name = buffer.read_string();
	auto graphics_name = buffer.read_string();
	auto found = find_first_true_unsorted(graphics_assets_map, [&graphics_name](const auto &pair){ return pair.first == graphics_name; });
	if (!found)
		throw std::runtime_error("TrainerClass::TrainerClass(): Static data error. Graphic " + graphics_name + " not found.");
	this->graphics = found->second;
	this->base_reward = buffer.read_varint();
	fill(this->ai_move_choice_modifiers, -1);
	for (std::uint32_t n = buffer.read_varint(), i = 0; n--; i++)
		this->ai_move_choice_modifiers[i] = buffer.read_varint();
	this->ai_class = buffer.read_string();
	this->ai_parameter = buffer.read_varint();
}

TrainerClassData::TrainerClassData(BufferReader &buffer): PartialTrainerClass(buffer){
	this->parties.reserve(buffer.read_varint());
	while (this->parties.size() < this->parties.capacity()){
		std::vector<TrainerPartyMember> party;
		party.reserve(buffer.read_varint());
		while (party.size() < party.capacity()){
			TrainerPartyMember member;
			member.species = (SpeciesId)buffer.read_varint();
			member.level = buffer.read_varint();
			party.push_back(member);
		}
		this->parties.emplace_back(std::move(party));
	}
}

const std::vector<TrainerPartyMember> &TrainerClassData::get_party(size_t index) const{
	if (index >= this->parties.size()){
		std::stringstream stream;
		stream << "TrainerClassData::get_party(): Invalid index " << index << ". Class " << this->name << " has " << this->parties.size() << " parties.";
		throw std::runtime_error(stream.str());
	}
	return this->parties[index];
}

FullTrainerClass TrainerClassData::get_trainer(size_t index) const{
	auto &party = this->get_party(index);
	return {*this, party};
}

const PartialTrainerClass &PartialTrainerClass::operator=(PartialTrainerClass &&other){
	this->id = other.id;
	this->name = std::move(other.name);
	this->display_name = std::move(other.display_name);
	this->graphics = other.graphics;
	this->base_reward = other.base_reward;
	std::copy(other.ai_move_choice_modifiers, other.ai_move_choice_modifiers + array_length(other.ai_move_choice_modifiers), this->ai_move_choice_modifiers);
	this->ai_class = std::move(other.ai_class);
	this->ai_parameter = other.ai_parameter;
	return *this;
}

const FullTrainerClass &FullTrainerClass::operator=(FullTrainerClass &&other){
	static_cast<PartialTrainerClass &>(*this) = std::move(static_cast<PartialTrainerClass &>(other));
	this->party = std::move(other.party);
	return *this;
}
