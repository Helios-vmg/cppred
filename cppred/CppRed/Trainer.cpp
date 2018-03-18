#include "Trainer.h"
#include <limits>

namespace CppRed{

NpcTrainer::NpcTrainer(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &instance):
		Npc(game, parent_coroutine, name, renderer, sprite, instance){
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
