#include "stdafx.h"
#include "PlayerCharacter.h"
#include "Game.h"
#include "Actor.h"
#include "../Maps.h"
#include "World.h"
#include "MainMenu.h"
#include "ItemFunctions.h"
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
	auto &map_instance = this->game->get_world().get_map_instance(destination.map);
	if (old_position.map != destination.map)
		this->entered_new_map(old_position.map, destination.map, true);
	else
		map_instance.set_cell_occupation(old_position.position, false);
	this->position = destination;
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
	while (!done && this->game->handle_standard_menu(options, callbacks))
		options.before_item_display = decltype(options.before_item_display)();
}

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

bool PlayerCharacter::display_inventory_menu(bool from_battle){
	bool ret = false;
	this->display_inventory_menu(this->inventory, [this, from_battle, &ret](const InventorySpace &is, int y){
		if (from_battle){
			ret = this->run_item_use_logic(is, from_battle) == InventoryChanges::Update;
			return ret ? InventoryChanges::Exit : InventoryChanges::Update;
		}
		UseTossResult use_toss;
		auto pusher = this->display_use_toss_dialog(use_toss, y);
		switch (use_toss){
			case UseTossResult::Use:
				{
					auto changes = this->run_item_use_logic(is, from_battle);
					ret = changes == InventoryChanges::Update;
					return changes;
				}
			case UseTossResult::Toss:
				return this->run_item_toss_logic(is, y);
			case UseTossResult::Cancel:
				return InventoryChanges::NoChange;
		}
		assert(false);
		return InventoryChanges::NoChange;
	});
	return ret;
}

PlayerCharacter::InventoryChanges PlayerCharacter::run_item_use_logic(const InventorySpace &is, bool from_battle){
	AutoRendererWindowPusher pusher(this->game->get_engine().get_renderer());
	auto &data = item_data[(int)is.item];
	auto result = data.use_function(data, *this->game, *this, from_battle);
	if (!result.was_used())
		return InventoryChanges::NoChange;
	this->inventory.remove(is.item, 1);
	return InventoryChanges::Update;
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

void draw_party(Renderer &renderer, Party &party, Tilemap &tilemap, std::vector<std::shared_ptr<Sprite>> &sprites, int hidden_start, int hidden_end){
	draw_party(renderer, party, tilemap, &sprites, hidden_start, hidden_end);
}

void draw_party(Renderer &renderer, Party &party, Tilemap &tilemap, std::vector<std::shared_ptr<Sprite>> *sprites, int hidden_start, int hidden_end){
	fill(tilemap.tiles, Tile());
	if (sprites)
		sprites->clear();
	int next_member_index = 0;
	for (Pokemon &p : party.iterate()){
		auto member_index = next_member_index++;
		if (member_index >= hidden_start && member_index <= hidden_end)
			continue;
		if (sprites){
			auto &data = p.get_data();
			auto sprite = renderer.create_sprite(2, 2);
			int i = 0,
				base_tile = MonPartySprites2.first_tile + (int)data.overworld_sprite * 2;
			for (SpriteTile &tile : sprite->iterate_tiles()){
				tile.tile_no = base_tile + i;
				i++;
				if (!(i % 2))
					i = i - 2 + MonPartySprites2.width;

			}
			sprite->set_position(Point(1, member_index * 2) * Renderer::tile_size);
			sprite->set_visible(true);
			sprites->push_back(std::move(sprite));

			sprite = renderer.create_sprite(2, 2);
			i = MonPartySprites2.width * 2;
			for (SpriteTile &tile : sprite->iterate_tiles()){
				tile.tile_no = base_tile + i;
				i++;
				if (!(i % 2))
					i = i - 2 + MonPartySprites2.width;
			}
			sprite->set_position(Point(1, member_index * 2) * Renderer::tile_size);
			sprites->push_back(sprite);
		}

		auto hp = p.get_current_hp();
		auto max_hp = p.get_max_hp();

		Point point(3, member_index * 2);
		renderer.draw_image_to_tilemap(point, PartyListHpBar);
		renderer.put_string(point, TileRegion::Background, p.get_display_name(), max_pokemon_name_size);
		renderer.draw_bar(point + Point(3, 1), TileRegion::Background, 6, max_hp, hp);
		renderer.put_string({14, member_index * 2}, TileRegion::Background, number_to_decimal_string(p.get_level()).data(), 3);
		renderer.put_string({13, member_index * 2 + 1}, TileRegion::Background, number_to_decimal_string(hp, 3).data());
		renderer.put_string({16, member_index * 2 + 1}, TileRegion::Background, "/");
		renderer.put_string({17, member_index * 2 + 1}, TileRegion::Background, number_to_decimal_string(max_hp, 3).data());
	}
}

bool PlayerCharacter::display_party_menu(PartyMenuOptions &options){
	auto &game = *this->game;
	auto &renderer = game.get_engine().get_renderer();
	auto &party = this->party;
	auto &coroutine = Coroutine::get_current_coroutine();
	auto &audio = game.get_audio_interface();
	AutoRendererPusher pusher;
	if (options.push_renderer)
		pusher = AutoRendererPusher(renderer);
	renderer.clear_screen();
	renderer.clear_sprites();
	renderer.set_palette(PaletteRegion::Sprites0, default_world_sprite_palette);

	std::vector<std::shared_ptr<Sprite>> sprites;
	sprites.reserve(Party::max_party_size * 2);

	int &selection = options.initial_item;

	auto &tilemap = renderer.get_tilemap(TileRegion::Background);
	auto &tiles = tilemap.tiles;
	{
		auto old = game.get_no_text_delay();
		game.set_no_text_delay(true);
		game.run_dialogue(options.prompt, false, false);
		game.set_no_text_delay(old);
	}
	game.reset_dialogue_state(false);

	bool redraw = true;
	bool sprite_visibility = false;
	auto first_switch_selection = options.first_switch_selection;

	game.reset_joypad_state();
	while (true){
		if (redraw){
			draw_party(renderer, party, tilemap, sprites);
			redraw = false;
			sprite_visibility = false;
		}
		for (int i = 0; i < party.size(); i++){
			auto &A = sprites[i * 2 + 0];
			auto &B = sprites[i * 2 + 1];
			bool active = i == selection;
			A->set_visible(!active | !sprite_visibility);
			B->set_visible(active & sprite_visibility);

			auto &tile = tiles[(i * 2 + 1) * Tilemap::w];
			if (active)
				tile.tile_no = black_arrow;
			else if (i == first_switch_selection)
				tile.tile_no = white_arrow;
			else
				tile.tile_no = ' ';
		}
		auto t0 = coroutine.get_clock().get();
		bool break_at_end = false;
		do{
			coroutine.yield();
			auto t1 = coroutine.get_clock().get();
			if (t1 - t0 >= 0.1){
				sprite_visibility = !sprite_visibility;
				break_at_end = true;
			}
			auto input = game.joypad_auto_repeat();
			if (input.get_b()){
				audio.play_sound(AudioResourceId::SFX_Press_AB);
				game.reset_joypad_state();
				return false;
			}
			if (input.get_down()){
				selection++;
				sprite_visibility = false;
				break;
			}
			if (input.get_up()){
				selection--;
				sprite_visibility = false;
				break;
			}
			if (input.get_a()){
				audio.play_sound(AudioResourceId::SFX_Press_AB);
				if (!options.callback){
					game.reset_joypad_state();
					return true;
				}
				auto result = options.callback(party.get(selection), selection);
				switch (result){
					case InventoryChanges::Exit:
						game.reset_joypad_state();
						return true;
					case InventoryChanges::Update:
						redraw = true;
					case InventoryChanges::NoChange:
						break;
					default:
						throw std::runtime_error("Bad switch.");
				}
				break;
			}
		}while (!break_at_end);
		selection = euclidean_modulo_u(selection, party.size());
	}
}

void PlayerCharacter::display_party_menu(){
	PartyMenuOptions options;
	options.callback = [this, &options](Pokemon &pokemon, int selection){
		return this->display_pokemon_actions_menu(pokemon, selection, options);
	};
	this->display_party_menu(options);
}

PlayerCharacter::InventoryChanges PlayerCharacter::display_pokemon_actions_menu(Pokemon &pokemon, int selection, PartyMenuOptions &options){
	auto &game = *this->game;
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	AutoRendererWindowPusher pusher(renderer);

	bool done = false;
	auto ret = InventoryChanges::NoChange;

	std::vector<std::string> items;
	std::vector<std::function<void()>> callbacks;

	//Populate with HM moves.

	items.push_back("STATS");
	callbacks.push_back([this, selection](){
		auto current = selection;
		while (true){
			switch (this->party.get(current).display_stats_screen(*this->game)){
				case StatsScreenResult::Close:
					break;
				case StatsScreenResult::GoToNext:
					current = (current + 1) % this->party.size();
					continue;
				case StatsScreenResult::GoToPrevious:
					current = euclidean_modulo_u(current - 1, this->party.size());
					continue;
				default:
					throw std::runtime_error("Bad switch.");
			}
			break;
		}
	});
	items.push_back("SWITCH");
	callbacks.push_back([this, selection, &ret, &options, &done](){
		auto result = this->do_party_switch(selection);
		ret = result >= 0 ? InventoryChanges::Update : InventoryChanges::NoChange;
		if (result >= 0)
			options.initial_item = result;
		done = true;
	});
	items.push_back("CANCEL");
	callbacks.push_back([&](){ done = true; });

	StandardMenuOptions options2;
	options2.position.x = Renderer::logical_screen_width - 1;
	options2.position.y = Renderer::logical_screen_height - 1;
	options2.items = &items;
	options2.push_window = false;

	while (!done && game.handle_standard_menu(options2, callbacks));

	return ret;
}

int PlayerCharacter::do_party_switch(int first_selection){
	int ret = -1;
	auto &party = this->party;
	
	PartyMenuOptions options;
	options.first_switch_selection = first_selection;
	options.callback = [&party, first_selection, &ret](Pokemon &pokemon, int selection){
		if (selection == first_selection)
			return InventoryChanges::Exit;
		int increment = selection > first_selection ? 1 : -1;
		for (int i = first_selection; i != selection; i += increment)
			std::swap(party.get(i), party.get(i + increment));
		ret = selection;

		return InventoryChanges::Exit;
	};
	options.prompt = TextResourceId::PartyMenuSwapMonText;
	options.initial_item = first_selection;
	options.push_renderer = false;

	auto &game = *this->game;
	auto &renderer = game.get_engine().get_renderer();
	AutoRendererPusher pusher(renderer);

	this->display_party_menu(options);

	if (ret >= 0){
		auto &audio = this->game->get_audio_interface();
		audio.stop_sfx();
		audio.play_sound(AudioResourceId::SFX_Swap);
		auto &coroutine = Coroutine::get_current_coroutine();
		std::vector<std::shared_ptr<Sprite>> sprites;
		auto &tilemap = renderer.get_tilemap(TileRegion::Background);
		draw_party(
			renderer,
			this->party,
			tilemap,
			sprites,
			std::min(first_selection, ret),
			std::max(first_selection, ret)
		);
		coroutine.wait(0.185);
		audio.stop_sfx();
		audio.play_sound(AudioResourceId::SFX_Swap);
	}
	return ret;
}

}
