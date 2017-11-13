#pragma once
#include "Pokemon.h"

struct InventorySpace{
	ItemId item;
	int quantity;
};

class CppRedTrainer{
public:
	static const size_t max_inventory_size;
private:
	std::string name;
	CppRedParty party;
	std::vector<InventorySpace> inventory;
public:
	CppRedTrainer() = default;
};
