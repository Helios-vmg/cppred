#pragma once

enum class ItemId;

namespace CppRed{

class Actor;
class Game;

typedef void (*item_use_function)(ItemId item_used, Game &game, Actor &user);

struct ItemData{
	ItemId id;
	const char *name;
	const char *display_name;
	int price;
	bool is_key;
	item_use_function use_function;
};

}
