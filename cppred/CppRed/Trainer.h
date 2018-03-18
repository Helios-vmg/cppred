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
public:
	virtual ~Trainer() = 0;
};

inline Trainer::~Trainer(){}

class NpcTrainer : public Npc, public Trainer{
public:
	NpcTrainer(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &instance);
};

}
