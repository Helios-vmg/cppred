#include "stdafx.h"
#include "PlayerCharacter.h"
#include "Game.h"
#include "Actor.h"
#include "../Maps.h"
#include "World.h"
#include "MainMenu.h"
#include "../../CodeGeneration/output/variables.h"
#ifndef HAVE_PCH
#include <cassert>
#include <sstream>
#include <iomanip>
#endif

namespace CppRed{

const size_t PlayerCharacter::max_pc_size = 50;
const Point PlayerCharacter::screen_block_offset(4, 4);
const PlayerCharacter::warp_check_f PlayerCharacter::warp_check_functions[2] = {
	&PlayerCharacter::facing_edge_of_map,
	&PlayerCharacter::is_in_front_of_warp_tile,
};

PlayerCharacter::PlayerCharacter(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer):
		Actor(game, parent_coroutine, name, renderer, RedSprite),
		Trainer(game.get_engine().get_prng()),
		pc_inventory(max_pc_size, max_inventory_item_quantity){
}

PlayerCharacter::~PlayerCharacter(){}

void PlayerCharacter::initialize_sprites(const GraphicsAsset &graphics, Renderer &renderer){
	Actor::initialize_sprites(graphics, renderer);
	auto s = Renderer::tile_size;
	Point position = screen_block_offset * (s * 2);
	position.y -= s / 2;
	this->apply_to_all_sprites([&position](auto &sprite){ sprite.set_position(position); });
}

void PlayerCharacter::teleport(const WorldCoordinates &destination){
	auto old_position = this->position;
	this->position = destination;
	auto &map_instance = this->game->get_world().get_map_instance(destination.map);
	if (old_position.map != destination.map)
		this->entered_new_map(old_position.map, destination.map, true);
	else
		map_instance.set_cell_occupation(old_position.position, false);
	map_instance.set_cell_occupation(destination.position, true);
}

void PlayerCharacter::coroutine_entry_point(){
	while (!this->quit_coroutine){
		if (!this->run_saved_actions())
			continue;

		if (this->ignore_input){
			this->coroutine->yield();
			continue;
		}

		auto input = this->game->get_engine().get_input_state();
		if (input.any_direction()){
			if (!this->move_internal(this->input_to_direction(input)))
				this->coroutine->yield();
			continue;
		}

		input = this->game->joypad_only_newly_pressed();
		if (input.get_start()){
			this->game->run_in_own_coroutine([this](){ this->display_menu(); });
		}else if (input.get_a()){
			MapObjectInstance *instances[8];
			auto &world = this->game->get_world();
			world.get_objects_at_location(instances, world.remap_coordinates(this->position + direction_to_vector(this->facing_direction)));
			bool activated = false;
			for (auto instance : instances){
				if (!instance)
					break;
				instance->activate(*this);
				activated = true;
			}
			if (!activated)
				this->check_for_bookshelf_or_card_key_door();
		}
		this->coroutine->yield();
	}
}

bool PlayerCharacter::move_internal(FacingDirection direction){
	if (Actor::move_internal(direction)){
		if (this->saved_post_warp){
			auto warp = this->saved_post_warp;
			this->saved_actions.push_back([this, warp](){
				auto &world = this->game->get_world();
				world.teleport_player(world.get_warp_destination(*warp));
			});
			this->saved_post_warp = nullptr;
		}
		return true;
	}
	if (!this->run_warp_logic_collision())
		return false;
	return Actor::move_internal(direction);
}

bool PlayerCharacter::run_warp_logic_collision(){
	auto &world = this->game->get_world();

	auto &ms = world.get_map_store();
	auto &map_data = ms.get_map_data(this->position.map);
	if (!(map_data.warp_check >= 0 && map_data.warp_check < array_length(this->warp_check_functions)))
		return false;
	if (!(this->*this->warp_check_functions[map_data.warp_check])(world))
		return false;

	MapObjectInstance *instances[8];
	world.get_objects_at_location(instances, this->position);
	const MapWarp *warp = nullptr;
	for (auto instance : instances){
		if (!instance)
			break;
		auto &o = instance->get_object();
		if (o.get_type() != MapObjectType::Warp)
			continue;
		warp = static_cast<const MapWarp *>(&o);
		break;
	}
	if (!warp)
		return false;
	auto destination = world.get_warp_destination(*warp);
	if (!this->can_move_to(destination, destination + direction_to_vector(this->facing_direction), this->facing_direction))
		return false;
	world.teleport_player(destination);
	return true;
}

bool PlayerCharacter::run_warp_logic_no_collision(){
	auto &world = this->game->get_world();
	auto &ms = world.get_map_store();
	auto &map_data = ms.get_map_data(this->position.map);
	auto &map_instance = world.get_map_instance(this->position.map);
	auto pos2 = this->position + direction_to_vector(this->facing_direction);
	
	MapObjectInstance *instances[8];
	world.get_objects_at_location(instances, pos2);
	const MapWarp *warp = nullptr;
	for (auto instance : instances){
		if (!instance)
			break;
		auto &o = instance->get_object();
		if (o.get_type() != MapObjectType::Warp)
			continue;
		if (!map_instance.is_warp_tile(pos2.position)){
			if (!(map_data.warp_check >= 0 && map_data.warp_check < array_length(this->warp_check_functions)))
				continue;
			if (!(this->*this->warp_check_functions[map_data.warp_check])(world))
				continue;
		}
		warp = static_cast<const MapWarp *>(&o);
		break;
	}
	if (!warp)
		return false;
	this->saved_post_warp = warp;
	return true;
}

FacingDirection PlayerCharacter::input_to_direction(const InputState &input){
	if (input.get_up())
		return FacingDirection::Up;
	if (input.get_right())
		return FacingDirection::Right;
	if (input.get_down())
		return FacingDirection::Down;
	assert(input.get_left());
	return FacingDirection::Left;
}

void PlayerCharacter::entered_new_map(Map old_map, Map new_map, bool warped){
	this->game->entered_map(old_map, new_map, warped);
}

bool PlayerCharacter::facing_edge_of_map(const World &world) const{
	return world.facing_edge_of_map(this->position, this->facing_direction);
}

bool PlayerCharacter::is_in_front_of_warp_tile(const World &world) const{
	auto pos2 = this->position + direction_to_vector(this->facing_direction);
	return world.get_map_instance(pos2.map).is_warp_tile(pos2.position);
}

void PlayerCharacter::about_to_move(){
	this->run_warp_logic_no_collision();
}

void PlayerCharacter::check_for_bookshelf_or_card_key_door(){
	auto pos2 = this->position + direction_to_vector(this->facing_direction);
	auto &map_data = this->game->get_world().get_map_store().get_map_data(pos2.map);
	auto tile = map_data.get_partial_tile_at_actor_position(pos2.position);
	auto info = map_data.tileset->get_bookcase_info(tile);
	if (!info)
		return;
	if (info->is_script)
		this->game->execute(info->script_name.c_str(), *this);
	else
		this->game->run_dialogue(info->text_id, true, true);
}

void PlayerCharacter::display_menu(){
	auto &coroutine = Coroutine::get_current_coroutine();
	auto &renderer = this->game->get_engine().get_renderer();
	auto &tilemap = renderer.get_tilemap(TileRegion::Window);
	bool done = false;

	std::vector<std::string> items;
	std::vector<std::function<void()>> callbacks;
	
	bool have_pokedex = this->game->get_variable_store().get(EventId::event_got_pokedex);

	if (have_pokedex){
		items.push_back("POK""\xE5""DEX");
		callbacks.push_back([](){});
	}
	
	if (this->party.size()){
		items.push_back("POK""\xE5""MON");
		callbacks.push_back([this](){
			return;
			this->display_party_menu();
		});
	}
	
	items.push_back("ITEM");
	callbacks.push_back([this](){
		this->display_inventory_menu();
	});
	
	items.push_back(this->name);
	callbacks.push_back([this](){
		this->display_player_menu();
	});
	
	items.push_back("SAVE");
	callbacks.push_back([this](){
		this->display_save_dialog();
	});
	
	items.push_back("OPTION");
	callbacks.push_back([this](){
		Scripts::show_options(*this->game);
	});
	
	items.push_back("EXIT");
	callbacks.push_back([&done](){ done = true; });
	
	StandardMenuOptions options;
	options.position = {Renderer::logical_screen_tile_width - 1, 0};
	options.items = &items;
	options.before_item_display = [this](){
		auto &audio = this->game->get_audio_interface();
		audio.play_sound(AudioResourceId::SFX_Start_Menu);
	};
	options.cancel_mask |= InputState::mask_start;
	options.push_window = false;
	AutoRendererWindowPusher pusher(this->game->get_engine().get_renderer());
	while (!done){
		auto input = this->game->handle_standard_menu(options);
		if (input < 0)
			break;
		callbacks[input]();
		options.before_item_display = decltype(options.before_item_display)();
	}
}

void PlayerCharacter::display_party_menu(){}

static std::array<char, 12> generate_inventory_quantity(int q){
	std::array<char, 12> ret;
	memset(ret.data(), ' ', 11);
	int i = 11;
	ret[i--] = 0;
	if (!q){
		ret[i--] = '0';
		ret[--i] = '*';
		return ret;
	}
	int digits = 0;
	for (; q; i--){
		if (i < 0){
			ret[0] = '*';
			return ret;
		}
		ret[i] = '0' + q % 10;
		q /= 10;
		digits++;
	}
	if (digits < 2)
		i--;
	ret[i] = '*';
	return ret;
}

void PlayerCharacter::display_inventory_menu(){
	this->display_inventory_menu(this->inventory, [this](const InventorySpace &is, int y){
		UseTossResult use_toss;
		auto pusher = this->display_use_toss_dialog(use_toss, y);
		switch (use_toss){
			case UseTossResult::Use:
				return this->run_item_use_logic(is);
			case UseTossResult::Toss:
				return this->run_item_toss_logic(is, y);
			case UseTossResult::Cancel:
				return InventoryChanges::NoChange;
		}
		assert(false);
		return InventoryChanges::NoChange;
	});
}

PlayerCharacter::InventoryChanges PlayerCharacter::run_item_use_logic(const InventorySpace &is){
	auto &data = item_data[(int)is.item];
	data.use_function(is.item, *this->game, *this);
	//TODO
	return InventoryChanges::NoChange;
}

PlayerCharacter::InventoryChanges PlayerCharacter::run_item_toss_logic(const InventorySpace& is, int y){
	auto &data = item_data[(int)is.item];
	auto &game = *this->game;
	auto &renderer = game.get_engine().get_renderer();
	if (data.is_key){
		AutoRendererWindowPusher pusher2(renderer);
		game.run_dialogue(TextResourceId::TooImportantToTossText, false, true);
		return InventoryChanges::NoChange;
	}
	int quantity;
	auto pusher2 = this->display_toss_quantity_dialog(quantity, is, y - 1);
	if (quantity < 1)
		return InventoryChanges::NoChange;
	auto &vs = game.get_variable_store();
	vs.set(StringVariableId::wcf4b_ThrowingAwayItemName, data.display_name);
	AutoRendererWindowPusher pusher3(renderer);
	game.run_dialogue(TextResourceId::IsItOKToTossItemText, false, false);
	bool ret = game.run_yes_no_menu(standard_dialogue_yes_no_position);
	if (!ret){
		game.reset_dialogue_state();
		return InventoryChanges::NoChange;
	}
	this->inventory.remove(is.item, quantity);
	vs.set(StringVariableId::wcd6d_item_stored_withdrawn, data.display_name);
	game.reset_dialogue_state();
	game.run_dialogue(TextResourceId::ThrewAwayItemText, false, true);
	return InventoryChanges::Update;
}

void PlayerCharacter::display_inventory_menu(Inventory &inventory, const std::function<InventoryChanges(const InventorySpace &, int)> &on_selection){
	StandardMenuOptions options;
	options.position = {4, 2};
	options.minimum_size = options.maximum_size = {16 - 2, 11 - 2};
	options.window_size = 4;
	options.push_window = false;
	while (true){
		std::vector<std::string> items;
		std::vector<std::string> quantities;
		for (auto &item : inventory.iterate_items()){
			auto &data = item_data[(int)item.item];
			items.push_back(data.display_name);
			if (data.is_key)
				quantities.emplace_back();
			else
				quantities.emplace_back(generate_inventory_quantity(item.quantity).data());
		}
		items.push_back("CANCEL");
		quantities.emplace_back();
		options.items = &items;
		options.extra_data = &quantities;
		AutoRendererWindowPusher pusher(this->game->get_engine().get_renderer());
		int selection;
		int passed_y;
		do{
			selection = this->game->handle_standard_menu(options);
			if (selection < 0 || selection == items.size() - 1)
				return;
			passed_y = options.position.y + 2 * (2 + options.initial_item - options.initial_window_position);
		}while (on_selection(inventory.get(selection), passed_y) == InventoryChanges::NoChange);
	}
}

void PlayerCharacter::display_player_menu(){}

void PlayerCharacter::display_save_dialog(){}

void Pokedex::set(FixedBitmap<pokemon_by_pokedex_id_size> &v, int &c, SpeciesId s){
	auto &data = *pokemon_by_species_id[(int)s];
	if (!data.allocated)
		return;
	auto index = (int)data.pokedex_id - 1;
	if (v.get(index))
		return;
	v.set(index);
	c++;
}

bool Pokedex::get(const FixedBitmap<pokemon_by_pokedex_id_size> &v, SpeciesId s){
	auto &data = *pokemon_by_species_id[(int)s];
	if (!data.allocated)
		return false;
	auto index = (int)data.pokedex_id - 1;
	return v.get(index);
}

void PlayerCharacter::open_pc(bool opened_at_home){
	this->game->run_dialogue(TextResourceId::WhatDoYouWantText, false, false);
	StandardMenuOptions options;
	std::vector<std::string> items;
	items.push_back("WITHDRAW ITEM");
	items.push_back("DEPOSIT ITEM");
	items.push_back("TOSS ITEM");
	items.push_back("LOG OFF");
	options.items = &items;
	options.push_window = false;
	AutoRendererWindowPusher pusher(this->game->get_engine().get_renderer());
	while (true){
		auto selection = this->game->handle_standard_menu(options);
		if (selection < 0)
			break;
		switch (selection){
			case 0:
				this->display_pc_withdraw_menu();
				break;
			case 1:
				this->display_pc_deposit_menu();
				break;
			case 2:
				this->display_pc_toss_menu();
				break;
			case 3:
				return;
		}
	}
}

AutoRendererWindowPusher PlayerCharacter::display_use_toss_dialog(UseTossResult &result, int y){
	StandardMenuOptions options;
	std::vector<std::string> items;
	items.push_back("USE");
	items.push_back("TOSS");
	options.items = &items;
	options.position = {13, y};
	options.push_window = false;
	options.initial_padding = false;
	AutoRendererWindowPusher pusher(this->game->get_engine().get_renderer());
	auto selection = this->game->handle_standard_menu(options);
	if (selection < 0)
		result = UseTossResult::Cancel;
	else if (!selection)
		result = UseTossResult::Use;
	else
		result = UseTossResult::Toss;
	return pusher;
}

void PlayerCharacter::display_pc_withdraw_menu(){
	InventoryTransferOptions options;
	options.src = &this->pc_inventory;
	options.dst = &this->inventory;
	options.nothing_to_do = TextResourceId::NothingStoredText;
	options.what_to_do = TextResourceId::WhatToWithdrawText;
	options.how_many = TextResourceId::WithdrawHowManyText;
	options.no_room = TextResourceId::CantCarryMoreText;
	options.done = TextResourceId::WithdrewItemText;
	this->display_inventory_transfer_menu(options);
}

void PlayerCharacter::display_pc_deposit_menu(){
	InventoryTransferOptions options;
	options.src = &this->inventory;
	options.dst = &this->pc_inventory;
	options.nothing_to_do = TextResourceId::NothingToDepositText;
	options.what_to_do = TextResourceId::WhatToDepositText;
	options.how_many = TextResourceId::DepositHowManyText;
	options.no_room = TextResourceId::NoRoomToStoreText;
	options.done = TextResourceId::ItemWasStoredText;
	this->display_inventory_transfer_menu(options);
}

void PlayerCharacter::display_pc_toss_menu(){
	InventoryTransferOptions options;
	options.src = &this->pc_inventory;
	options.dst = nullptr;
	options.nothing_to_do = TextResourceId::NothingStoredText;
	options.what_to_do = TextResourceId::WhatToTossText;
	options.how_many = TextResourceId::TossHowManyText;
	options.no_room = TextResourceId::ExclamationText;
	options.done = TextResourceId::ThrewAwayItemText;
	this->display_inventory_transfer_menu(options);
}

void PlayerCharacter::display_inventory_transfer_menu(const InventoryTransferOptions &options){
	auto &game = *this->game;
	auto &renderer = game.get_engine().get_renderer();
	AutoRendererWindowPusher pusher1(renderer);
	game.reset_dialogue_state(false);
	if (options.src->empty()){
		game.run_dialogue(options.nothing_to_do, false, false);
		return;
	}
	game.run_dialogue(options.what_to_do, false, false);
	this->display_inventory_menu(*options.src, [this, &game, &renderer, &options](const InventorySpace &is, int y){
		AutoRendererWindowPusher pusher2(renderer);
		game.reset_dialogue_state(false);
		game.run_dialogue(options.how_many, false, false);
		int quantity;
		{
			GetQuantityFromUserOptions options2;
			options2.position = standard_dialogue_quantity_position;
			options2.min = 1;
			options2.max = is.quantity;
			options2.minimum_digits = 2;
			options2.push_window = true;
			quantity = game.get_quantity_from_user(options2);
		}
		if (quantity < 1)
			return InventoryChanges::NoChange;
		auto &data = item_data[(int)is.item];
		auto &vs = game.get_variable_store();
		if (options.dst){
			if (!options.dst->receive(is.item, quantity)){
				game.reset_dialogue_state(false);
				game.run_dialogue(TextResourceId::NoRoomToStoreText, false, false);
				return InventoryChanges::NoChange;
			}
			options.src->remove(is.item, quantity);
			game.get_audio_interface().play_sound(AudioResourceId::SFX_Withdraw_Deposit);
		}else{
			game.reset_dialogue_state(false);
			if (data.is_key){
				game.run_dialogue(TextResourceId::TooImportantToTossText, false, false);
				return InventoryChanges::NoChange;
			}
			vs.set(StringVariableId::wcf4b_ThrowingAwayItemName, data.display_name);
			game.run_dialogue(TextResourceId::IsItOKToTossItemText, false, false);
			if (!game.run_yes_no_menu(standard_dialogue_yes_no_position))
				return InventoryChanges::NoChange;
			options.src->remove(is.item, quantity);
		}
		game.reset_dialogue_state(false);
		vs.set(StringVariableId::wcd6d_item_stored_withdrawn, data.display_name);
		game.run_dialogue(options.done, false, false);
		return InventoryChanges::Update;
	});
}

AutoRendererWindowPusher PlayerCharacter::display_toss_quantity_dialog(int &result, const InventorySpace &is, int y){
	GetQuantityFromUserOptions options;
	options.min = 1;
	options.max = is.quantity;
	options.position = {Renderer::logical_screen_tile_width, y};
	options.minimum_digits = 2;
	options.push_window = false;
	AutoRendererWindowPusher pusher(this->game->get_engine().get_renderer());
	result = this->game->get_quantity_from_user(options);
	return pusher;
}

}
