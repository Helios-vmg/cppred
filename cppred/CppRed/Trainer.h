#pragma once
#include "Pokemon.h"
#include "Actor.h"

namespace CppRed{

struct InventorySpace{
	ItemId item;
	int quantity;
};

class Trainer : public Actor{
public:
	static const size_t max_inventory_size;
protected:
	Party party;
	std::vector<InventorySpace> inventory;
	virtual void coroutine_entry_point() override{}
public:
	Trainer(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite);
	virtual ~Trainer(){}
};

}
