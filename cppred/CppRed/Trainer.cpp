#include "stdafx.h"
#include "Trainer.h"
#include "Game.h"
#include "Engine.h"
#ifndef HAVE_PCH
#include <limits>
#endif

namespace CppRed{

const size_t Trainer::max_inventory_size = 20;
const int Trainer::max_inventory_item_quantity = 99;

Trainer::Trainer(XorShift128 &rng):
		inventory(max_inventory_size, max_inventory_item_quantity){
	typedef decltype(this->trainer_id) T;
	this->trainer_id = (T)rng((std::uint32_t)std::numeric_limits<T>::max() + 1);
}

Trainer::~Trainer(){}

NpcTrainer::NpcTrainer(
	Game &game,
	Coroutine &parent_coroutine,
	const std::string &name,
	Renderer &renderer,
	const GraphicsAsset &sprite,
	MapObjectInstance &instance,
	const std::shared_ptr<TrainerClassData> &trainer_class,
	int default_party):
		Npc(game, parent_coroutine, name, renderer, sprite, instance),
		Trainer(game.get_engine().get_prng()),
		trainer_class(trainer_class),
		default_party(default_party){
}

NpcTrainer::~NpcTrainer(){}

FullTrainerClass NpcTrainer::get_party(int index){
	if (index < 0)
		index = this->default_party;
	return this->trainer_class->get_trainer(index);
}

Inventory::Inventory(size_t max_size, int max_item_quantity){
	this->max_size = max_size;
	this->max_item_quantity = max_item_quantity;
	this->inventory.reserve(max_size);
}

bool Inventory::contains(ItemId item) const{
	for (auto &i : this->inventory)
		if (i.item == item)
			return true;
	return false;
}

bool Inventory::receive(ItemId item, int quantity){
	for (auto &i : this->inventory){
		if (i.item == item){
			if (this->max_item_quantity - quantity < i.quantity)
				return false;
			i.quantity += quantity;
			return true;
		}
	}
	if (this->inventory.size() >= this->max_size)
		return false;
	this->inventory.emplace_back(InventorySpace{ item, quantity });
	return true;
}

void Inventory::remove(ItemId item, int quantity){
	for (auto i = this->inventory.begin(), e = this->inventory.end(); i != e; ++i){
		if (i->item != item)
			continue;
		if (quantity < 0 || quantity >= i->quantity)
			this->inventory.erase(i);
		else
			i->quantity -= quantity;
		break;
	}
}

iterator_range<std::vector<InventorySpace>::iterator> Inventory::iterate_items(){
	return iterator_range<std::vector<InventorySpace>::iterator>(this->inventory.begin(), this->inventory.end());
}

bool Inventory::empty() const{
	return !this->inventory.size();
}

}
