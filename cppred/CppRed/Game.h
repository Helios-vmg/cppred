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
#include <string>
#include <unordered_map>
#include <queue>

struct MapData;

namespace CppRed{

class Trainer;
class PlayerCharacter;
class World;
enum class EventId;
enum class VisibilityFlagId;
enum class IntegerVariableId;
enum class StringVariableId;

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
	bool dialog_box_visible = false;
	VariableStore variable_store;
	AudioInterface audio_interface;
	std::unique_ptr<World> world;
	bool reset_dialog_was_delayed = false;
	bool no_text_delay = false;
	std::unique_ptr<Coroutine> coroutine;

	void update_joypad_state();
	bool check_for_user_interruption_internal(bool autorepeat, double timeout, InputState *);
	std::string get_name_from_user(NameEntryType, SpeciesId, int max_length);
	template <typename T, typename ...Params>
	void switch_screen_owner(Actor &actor, Params &&...params){
		std::unique_ptr<ScreenOwner> p(new T(*this, std::forward<Params>(params)...));
		actor.set_new_screen_owner(std::move(p));
		Coroutine::get_current_coroutine().yield();
	}
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
	void palette_whiteout();
	bool check_for_user_interruption(double timeout = 0, InputState * = nullptr);
	bool check_for_user_interruption_no_auto_repeat(double timeout = 0, InputState * = nullptr);
	bool check_for_user_interruption(InputState &is){
		return this->check_for_user_interruption(0, &is);
	}
	InputState joypad_auto_repeat();
	InputState joypad_only_newly_pressed();
	void wait_for_sound_to_finish();
	bool run_yes_no_dialog(const Point &point);
	typedef decltype(SavableData::load("")) load_save_t;
	load_save_t load_save();
	void draw_box(const Point &corner, const Point &size, TileRegion);
	int handle_standard_menu(
		TileRegion region,
		const Point &position,
		const std::vector<std::string> &items,
		const Point &minimum_size = { 0, 0 },
		bool ignore_b = false
	);
	int handle_standard_menu_with_title(
		TileRegion region,
		const Point &position,
		const std::vector<std::string> &items,
		const char *title,
		const Point &minimum_size = { 0, 0 },
		bool ignore_b = false
	);
	void put_string(const Point &position, TileRegion region, const char *string, int pad_to = 0);
	void run_dex_entry_from_script(TextResourceId);
	void run_dialog(TextResourceId, bool wait_at_end = false);
	void run_dialog_from_script(TextResourceId text, bool wait_at_end = true){
		this->run_dialog(text, wait_at_end);
	}
	void run_dialog_from_world(TextResourceId, Actor &activator, bool hide_window_at_end = true);
	void reset_dialog_state();
	void delayed_reset_dialog(){
		this->reset_dialog_was_delayed = true;
	}
	static TextState get_default_dialog_state();
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
	void display_pokedex_page(PokedexId, Actor &requester);
	void draw_portrait(const GraphicsAsset &, TileRegion, const Point &corner, bool flipped = false);

	DEFINE_GETTER_SETTER(options)
	DEFINE_GETTER_SETTER(options_initialized)
};

}
