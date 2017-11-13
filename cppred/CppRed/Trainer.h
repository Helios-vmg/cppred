#pragma once
#include "Pokemon.h"

namespace CppRed{

struct InventorySpace{
	ItemId item;
	int quantity;
};

class Trainer{
public:
	static const size_t max_inventory_size;
private:
	std::string name;
	Party party;
	std::vector<InventorySpace> inventory;
public:
	Trainer() = default;
};

}
