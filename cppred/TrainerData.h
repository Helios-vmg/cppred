#pragma once
#include <array>

enum class SpeciesId;

struct TrainerPartyMember{
	SpeciesId species;
	int level;
};

class BaseTrainerParty{
public:
	virtual ~BaseTrainerParty(){}
	virtual size_t get_length() const = 0;
	virtual TrainerPartyMember get_member(size_t i) const = 0;
};

template <size_t N>
class TrainerParty : public BaseTrainerParty{
	std::array<TrainerPartyMember, N> members;
public:
	TrainerParty(std::array<TrainerPartyMember, N> &&members): members(std::move(members)){}
	size_t get_length() const{
		return N;
	}
	TrainerPartyMember get_member(size_t i) const{
		return this->members[i];
	}
};
