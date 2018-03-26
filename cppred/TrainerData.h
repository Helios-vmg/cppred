#pragma once
#ifndef HAVE_PCH
#include <array>
#include <memory>
#endif

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
	virtual TrainerPartyMember &get_member(size_t i) = 0;
};

template <size_t N>
class TrainerParty : public BaseTrainerParty{
	std::array<TrainerPartyMember, N> members;
public:
	TrainerParty() = default;
	TrainerParty(std::array<TrainerPartyMember, N> &&members): members(std::move(members)){}
	size_t get_length() const override{
		return N;
	}
	TrainerPartyMember get_member(size_t i) const override{
		return this->members[i];
	}
	TrainerPartyMember &get_member(size_t i) override{
		return this->members[i];
	}
};

template <size_t lo, size_t hi>
static std::shared_ptr<BaseTrainerParty> allocate_TrainerParty(size_t count){
	constexpr size_t pivot = (hi - lo) / 2 + lo;
	if (count < pivot)
		return allocate_TrainerParty<lo, pivot>(count);
	if (count > pivot)
		return allocate_TrainerParty<pivot, hi>(count);
	return std::make_shared<TrainerParty<pivot>>();
}
