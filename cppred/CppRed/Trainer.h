#pragma once
#include "Pokemon.h"
#include "Actor.h"
#include "Npc.h"

namespace CppRed{

struct InventorySpace{
	ItemId item;
	int quantity;
};

class Trainer{
public:
	static const size_t max_inventory_size;
protected:
	Party party;
	std::vector<InventorySpace> inventory;
	std::uint16_t trainer_id;
public:
	Trainer(XorShift128 &);
	virtual ~Trainer() = 0;
	bool has_item_in_inventory(ItemId) const;
	void receive(ItemId, int);
	void remove_all(ItemId);
	Party &get_party(){
		return this->party;
	}
	const Party &get_party() const{
		return this->party;
	}
	DEFINE_GETTER(trainer_id)
};

inline Trainer::~Trainer(){}

class NpcTrainer : public Npc, public Trainer{
	std::map<int, std::shared_ptr<BaseTrainerParty>> parties;
	int default_party;
public:
	NpcTrainer(
		Game &game,
		Coroutine &parent_coroutine,
		const std::string &name,
		Renderer &renderer,
		const GraphicsAsset &sprite,
		MapObjectInstance &instance,
		const std::map<int, std::shared_ptr<BaseTrainerParty>> &,
		int default_party);
	const BaseTrainerParty &get_party(int index = -1);
};

}
