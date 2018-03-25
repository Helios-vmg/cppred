#include "Trainer.h"
#include "Game.h"
#include "Engine.h"
#include <limits>

namespace CppRed{

const size_t Trainer::max_inventory_size = 20;
const int Trainer::max_inventory_item_quantity = 99;

Trainer::Trainer(XorShift128 &rng){
	typedef decltype(this->trainer_id) T;
	this->trainer_id = (T)rng((std::uint32_t)std::numeric_limits<T>::max() + 1);
}

NpcTrainer::NpcTrainer(
	Game &game,
	Coroutine &parent_coroutine,
	const std::string &name,
	Renderer &renderer,
	const GraphicsAsset &sprite,
	MapObjectInstance &instance,
	const std::map<int, std::shared_ptr<BaseTrainerParty>> &parties,
	int default_party):
		Npc(game, parent_coroutine, name, renderer, sprite, instance),
		Trainer(game.get_engine().get_prng()),
		parties(parties),
		default_party(default_party){
}

const BaseTrainerParty &NpcTrainer::get_party(int index){
	if (index < 0)
		index = this->default_party;
	return *this->parties[index];
}

bool Trainer::has_item_in_inventory(ItemId item) const{
	for (auto &i : this->inventory)
		if (i.item == item)
			return true;
	return false;
}

void  Trainer::receive(ItemId item, int quantity){
	const auto max = std::numeric_limits<int>::max();
	for (auto &i : this->inventory){
		if (i.item == item){
			if (max - quantity < i.quantity)
				i.quantity = max;
			else
				i.quantity += quantity;
			return;
		}
	}
	this->inventory.push_back({item, quantity});
}

void Trainer::remove_all(ItemId item){
	for (auto i = this->inventory.begin(), e = this->inventory.end(); i != e; ++i){
		if (i->item != item)
			continue;
		this->inventory.erase(i);
		return;
	}
}

}
