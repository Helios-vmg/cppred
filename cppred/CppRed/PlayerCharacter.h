#pragma once
#include "Trainer.h"
#include "Renderer.h"
#include "utility.h"

class InputState;
class MapWarp;
enum class Map;

namespace CppRed{

class Game;
class World;

class Pokedex{
	FixedBitmap<pokemon_by_pokedex_id_size> seen;
	FixedBitmap<pokemon_by_pokedex_id_size> owned;
	int seen_count = 0;
	int owned_count = 0;

	void set(FixedBitmap<pokemon_by_pokedex_id_size> &, int &, SpeciesId);
	bool get(const FixedBitmap<pokemon_by_pokedex_id_size> &, SpeciesId);
public:
	Pokedex() = default;
	Pokedex(const Pokedex &) = default;
	Pokedex(Pokedex &&) = delete;
	void set_seen(SpeciesId species){
		this->set(this->seen, this->seen_count, species);
	}
	bool get_seen(SpeciesId species){
		return this->get(this->seen, species);
	}
	void set_owned(SpeciesId species){
		this->set_seen(species);
		this->set(this->owned, this->owned_count, species);
	}
	bool get_owned(SpeciesId species){
		return this->get(this->owned, species);
	}
	DEFINE_GETTER(seen_count)
	DEFINE_GETTER(owned_count)
};

class PlayerCharacter : public Actor, public Trainer{
public:
	static const Point screen_block_offset;
	static const size_t max_pc_size;
private:
	typedef bool (PlayerCharacter::*warp_check_f)(const World &) const;
	static const warp_check_f warp_check_functions[2];
	const MapWarp *saved_post_warp = nullptr;
	bool ignore_input = false;
	Pokedex pokedex;
	Inventory pc_inventory;

	void coroutine_entry_point() override;
	void entered_new_map(Map old_map, Map new_map, bool warped) override;
	void initialize_sprites(const GraphicsAsset &graphics, Renderer &) override;
	bool run_warp_logic_no_collision();
	bool run_warp_logic_collision();
	bool facing_edge_of_map(const World &) const;
	bool is_in_front_of_warp_tile(const World &) const;
	void about_to_move() override;
	static FacingDirection input_to_direction(const InputState &input);
	bool move_internal(FacingDirection) override;
	void check_for_bookshelf_or_card_key_door();
	void display_menu();
	void display_party_menu();
	void display_inventory_menu();
	enum class InventoryChanges{
		NoChange,
		Update,
	};
	void display_inventory_menu(Inventory &, const std::function<InventoryChanges(const InventorySpace &, int)> &);
	void display_player_menu();
	void display_save_dialog();
	enum class UseTossResult{
		Use,
		Toss,
		Cancel,
	};
	AutoRendererWindowPusher display_use_toss_dialog(UseTossResult &, int);
	void display_pc_withdraw_menu();
	void display_pc_deposit_menu();
	struct InventoryTransferOptions{
		Inventory *dst;
		Inventory *src;
		TextResourceId nothing_to_do;
		TextResourceId what_to_do;
		TextResourceId how_many;
		TextResourceId no_room;
		TextResourceId done;
	};
	void display_inventory_transfer_menu(const InventoryTransferOptions &options);
	void display_pc_toss_menu();
	AutoRendererWindowPusher display_toss_quantity_dialog(int &result, const InventorySpace &, int);
	InventoryChanges run_item_use_logic(const InventorySpace &is);
	InventoryChanges run_item_toss_logic(const InventorySpace &is, int y);
public:
	PlayerCharacter(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &);
	~PlayerCharacter();
	void teleport(const WorldCoordinates &);
	DEFINE_GETTER_SETTER(ignore_input)
	DEFINE_NON_CONST_GETTER(pc_inventory)
	Pokedex &get_pokedex(){
		return this->pokedex;
	}
	void open_pc(bool opened_at_home);
};

class AutoIgnoreInput{
	PlayerCharacter *pc;
public:
	AutoIgnoreInput(PlayerCharacter &pc): pc(&pc){
		this->pc->set_ignore_input(true);
	}
	AutoIgnoreInput(const AutoIgnoreInput &) = delete;
	AutoIgnoreInput(AutoIgnoreInput &&other){
		*this = std::move(other);
	}
	~AutoIgnoreInput(){
		if (this->pc)
			this->pc->set_ignore_input(false);
	}
	const AutoIgnoreInput &operator=(const AutoIgnoreInput &) = delete;
	const AutoIgnoreInput &operator=(AutoIgnoreInput &&other){
		if (this->pc)
			this->pc->set_ignore_input(false);
		this->pc = other.pc;
		other.pc = nullptr;
		return *this;
	}
};

}
