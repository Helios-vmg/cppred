#pragma once
#include "Pokemon.h"
#include "Actor.h"
#include "Npc.h"
#include "../utility.h"

namespace CppRed{

struct InventorySpace{
	ItemId item;
	int quantity;
};

class Inventory{
	std::vector<InventorySpace> inventory;
	size_t max_size;
	int max_item_quantity;
public:
	Inventory(size_t max_size, int max_item_quantity);
	bool receive(ItemId, int);
	void remove(ItemId, int quantity = -1);
	bool contains(ItemId) const;
	iterator_range<std::vector<InventorySpace>::iterator> iterate_items();
	InventorySpace get(size_t i){
		return this->inventory[i];
	}
	bool empty() const;
};

class Trainer{
public:
	static const size_t max_inventory_size;
	static const int max_inventory_item_quantity;
protected:
	Party party;
	Inventory inventory;
	std::uint16_t trainer_id;
public:
	Trainer(XorShift128 &);
	virtual ~Trainer() = 0;
	Party &get_party(){
		return this->party;
	}
	const Party &get_party() const{
		return this->party;
	}
	DEFINE_GETTER(trainer_id)
	DEFINE_NON_CONST_GETTER(inventory)
};

class NpcTrainer : public Npc, public Trainer{
	std::shared_ptr<TrainerClassData> trainer_class;
	int default_party;
public:
	NpcTrainer(
		Game &game,
		Coroutine &parent_coroutine,
		const std::string &name,
		Renderer &renderer,
		const GraphicsAsset &sprite,
		MapObjectInstance &instance,
		const std::shared_ptr<TrainerClassData> &trainer_class,
		int default_party);
	~NpcTrainer();
	FullTrainerClass get_party(int index = -1);
};

}
