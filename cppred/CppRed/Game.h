#pragma once
#include "Engine.h"
#include "utility.h"
#include "Data.h"
#include "SavableData.h"
#include "TextResources.h"
#include "pokemon_version.h"
#include "AudioInterface.h"
#include "ScreenOwner.h"
#include "Actor.h"
#ifndef HAVE_PCH
#include <string>
#include <queue>
#endif

struct MapData;
class Coroutine;

namespace CppRed{

const int max_pokemon_name_size = 10;
const int max_character_name_size = 7;

class Trainer;
class NpcTrainer;
class PlayerCharacter;
class World;
enum class EventId;
enum class VisibilityFlagId;
enum class IntegerVariableId;
enum class StringVariableId;
enum class BattleResult;

class VariableStore{
	std::vector<std::string> strings;
	std::vector<int> integers;
	std::vector<bool> events;
	std::vector<bool> vibility_flags;
	template <int Offset, typename T1, typename T2>
	static void set_vector(std::vector<T1> &v, T2 s, const T1 &value){
		size_t k = (size_t)s - Offset;
		if (v.size() < k + 1)
			v.resize(k + 1);
		v[k] = value;
	}
	template <int Offset, typename T1, typename T2>
	static const T1 &get_vector(std::vector<T1> &v, T2 s){
		size_t k = (size_t)s - Offset;
		if (v.size() < k + 1)
			v.resize(k + 1);
		return v[k];
	}
	template <int Offset, typename T2>
	static bool get_vector(std::vector<bool> &v, T2 s){
		size_t k = (size_t)s - Offset;
		if (v.size() < k + 1)
			v.resize(k + 1);
		return v[k];
	}
public:
	void set(StringVariableId, const std::string &);
	void set(IntegerVariableId, int);
	void set(EventId, bool);
	void set(VisibilityFlagId, bool);
	const std::string &get(StringVariableId);
	int get(IntegerVariableId);
	bool get(EventId);
	bool get(VisibilityFlagId);
	void load_initial_visibility_flags();
};

class Game;

enum class NameEntryType{
	Player,
	Rival,
	Pokemon,
};

enum class GameState{
	World,
	Battle,
	Menu,
	TextDisplay,
};

const Point standard_dialogue_yes_no_position(14, 7);
const Point standard_dialogue_quantity_position(Renderer::logical_screen_tile_width, 9);
const Point standard_dialogue_box_position(0, Renderer::logical_screen_tile_height - 6);
const Point standard_dialogue_box_size(Renderer::logical_screen_tile_width - 2, 4);

enum class MenuAnchor{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
};

struct StandardMenuOptions{
	static const int int_max = std::numeric_limits<int>::max();

	Point position;
	const std::vector<std::string> *items = nullptr;
	const std::vector<std::string> *extra_data = nullptr;
	const char *title = nullptr;
	Point minimum_size;
	Point maximum_size = {int_max, int_max};
	bool ignore_b = false;
	bool initial_padding = true;
	MenuAnchor anchor = MenuAnchor::TopLeft;
	std::function<void()> before_item_display;
	int initial_item = 0;
	int initial_window_position = 0;
	byte_t cancel_mask = InputState::mask_b;
	int window_size = int_max;
	bool push_window = true;
	int item_spacing = 2;
	std::function<void(int)> on_item_hover;
};

struct GetQuantityFromUserOptions{
	static const int int_max = std::numeric_limits<int>::max();
	
	Point position;
	int min = 0;
	int max = int_max;
	bool push_window = true;
	bool ignore_b = false;
	int minimum_digits = 0;
};

class Game{
	Engine *engine;
	PokemonVersion version;
	TextStore text_store;
	InputState joypad_held;
	InputState joypad_pressed;
	double jls_timeout = std::numeric_limits<double>::max();
	GameOptions options;
	bool options_initialized = false;
	TextState text_state;
	bool dialogue_box_visible = false;
	bool dialogue_box_clear_required = true;
	VariableStore variable_store;
	AudioInterface audio_interface;
	std::unique_ptr<World> world;
	bool reset_dialogue_was_delayed = false;
	bool no_text_delay = false;
	std::unique_ptr<Coroutine> coroutine;
	std::vector<std::shared_ptr<ScreenOwner>> owners_stack;
	int locks_acquired = 0;

	void update_joypad_state();
	bool check_for_user_interruption_internal(bool autorepeat, double timeout, InputState *);
	std::string get_name_from_user(NameEntryType, SpeciesId, int max_length);
	template <typename T, typename ...Params>
	std::shared_ptr<T> switch_screen_owner(Params &&...params){
		auto ret = std::make_shared<T>(*this, std::forward<Params>(params)...);
		this->owners_stack.push_back(ret);
		return ret;
		//Coroutine::get_current_coroutine().yield();
	}
	template <typename T, typename ...Params>
	bool check_screen_owner(bool require_unlock, Params &&...params){
		if (require_unlock && !this->locks_acquired)
			return false;
		this->switch_screen_owner<T>(std::forward<Params>(params)...);
		auto c = Coroutine::get_current_coroutine_ptr();
		if (c != this->coroutine.get())
			c->yield();
		return true;
	}
	bool update_internal();
	ScreenOwner *get_current_owner();
	std::array<std::shared_ptr<Sprite>, 2> load_mon_sprites(SpeciesId species);
public:
	Game(Engine &engine, PokemonVersion version, CppRed::AudioProgramInterface &program);
	Game(Game &&) = delete;
	Game(const Game &) = delete;
	void operator=(Game &&) = delete;
	void operator=(const Game &) = delete;
	~Game();
	void update();
	void clear_screen();
	Engine &get_engine(){
		return *this->engine;
	}
	void fade_out_to_white();
	void fade_out_to_black();
	void palette_whiteout();
	void palette_blackout();
	bool check_for_user_interruption(double timeout = 0, InputState * = nullptr);
	bool check_for_user_interruption_no_auto_repeat(double timeout = 0, InputState * = nullptr);
	bool check_for_user_interruption(InputState &is){
		return this->check_for_user_interruption(0, &is);
	}
	InputState joypad_auto_repeat();
	InputState joypad_only_newly_pressed();
	void wait_for_sound_to_finish();
	bool run_yes_no_menu(const Point &point);
	typedef decltype(SavableData::load("")) load_save_t;
	load_save_t load_save();
	void draw_box(const Point &corner, const Point &size, TileRegion);
	int handle_standard_menu(StandardMenuOptions &);
	bool handle_standard_menu(StandardMenuOptions &, const std::vector<std::function<void()>> &callbacks);
	void run_dex_entry(TextResourceId);
	void run_dialogue(TextResourceId, bool wait_at_end, bool hide_dialogue_at_end);
	void reset_dialogue_state(bool also_hide_window = true);
	void delayed_reset_dialogue(){
		this->reset_dialogue_was_delayed = true;
	}
	static TextState get_default_dialogue_state();
	void text_print_delay();
	VariableStore &get_variable_store(){
		return this->variable_store;
	}
	std::string get_name_from_user(NameEntryType, int max_length = -1);
	std::string get_name_from_user(SpeciesId, int max_length = -1);
	PokemonVersion get_version() const{
		return this->version;
	}
	CppRed::AudioInterface &get_audio_interface(){
		return this->audio_interface;
	}
	void create_main_characters(const std::string &player_name, const std::string &rival_name);
	void game_loop();
	void entered_map(Map old_map, Map new_map, bool warped);
	void teleport_player(const WorldCoordinates &);
	World &get_world(){
		return *this->world;
	}
	void execute(const char *script_name, Actor &caller, const char *parameter = nullptr);
	Coroutine &get_coroutine(){
		return *this->coroutine;
	}
	void display_pokedex_page(PokedexId);
	void draw_portrait(const GraphicsAsset &, TileRegion, const Point &corner, bool flipped = false);
	bool run_in_own_coroutine(std::function<void()> &&, bool synchronous = true);

	DEFINE_GETTER_SETTER(options)
	DEFINE_GETTER_SETTER(options_initialized)
	DEFINE_GETTER_SETTER(no_text_delay)
	void lock(){
		this->locks_acquired++;
	}
	void unlock(){
		this->locks_acquired--;
	}
	void dialogue_wait();
	BattleResult run_trainer_battle(TextResourceId player_victory_text, TextResourceId player_defeat_text, FullTrainerClass &&);
	int get_quantity_from_user(const GetQuantityFromUserOptions &);
	void reset_joypad_state();
};

}
