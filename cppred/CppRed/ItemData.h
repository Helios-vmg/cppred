#pragma once

enum class ItemId;

namespace CppRed{

class Game;
class PlayerCharacter;
struct ItemData;
struct ItemFunctionParams;

enum class ItemUseError{
	NoError,
	UseCancelled,
	NotImplemented,
	EmptyParty,
	NoEffect,
};

class ItemUseResult{
	bool success;
	bool used;
	ItemUseError error;
public:
	explicit ItemUseResult(): success(true), used(true), error(ItemUseError::NoError){}
	explicit ItemUseResult(ItemUseError error): success(false), used(false), error(error){
		switch (error){
			case ItemUseError::UseCancelled:
				this->success = true;
				break;
			default:
				break;
		}
	}
	//explicit ItemUseResult(bool success, bool used, ItemUseError error): success(success), used(used), error(error){}
	bool succeeded() const{
		return this->success;
	}
	bool was_used() const{
		return this->used;
	}
	ItemUseError get_error() const{
		return this->error;
	}
};

typedef ItemUseResult (*item_use_function)(const ItemData &item_used, Game &game, PlayerCharacter &user, bool from_battle);

struct ItemData{
	ItemId id;
	const char *name;
	const char *display_name;
	int price;
	bool is_key;
	item_use_function use_function;
	int function_parameter;
};

}
