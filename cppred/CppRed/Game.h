#pragma once
#include "Engine.h"
#include "utility.h"
#include "Data.h"
#include "SavableData.h"
#include "TextResources.h"
#include "pokemon_version.h"
#include "AudioInterface.h"
#include "Maps.h"
#include <string>
#include <unordered_map>
#include <queue>

struct MapData;

namespace CppRed{

class Trainer;
class PlayerCharacter;

class VariableStore{
	std::unordered_map<std::string, std::string *> string_variables;
	std::unordered_map<std::string, int *> number_variables;
	std::deque<std::string> strings;
	std::deque<int> numbers;
public:
	void set_string(const std::string &key, std::string *value);
	void set_string(const std::string &key, const std::string &value);
	void set_number(const std::string &key, int *value);
	void set_number(const std::string &key, int value);
	const std::string &get_string(const std::string &key);
	int get_number(const std::string &key);
	void delete_string(const std::string &key);
	void delete_number(const std::string &key);
};

enum class NameEntryType{
	Player,
	Rival,
	Pokemon,
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
	std::unique_ptr<PlayerCharacter> player_character;
	std::unique_ptr<Trainer> rival;
	MapStore map_store;

	void update_joypad_state();
	bool check_for_user_interruption_internal(bool autorepeat, double timeout, InputState *);
	std::string get_name_from_user(NameEntryType, SpeciesId, int max_length);
	void render();
public:
	Game(Engine &engine, PokemonVersion version, CppRed::AudioProgram &program);
	Game(Game &&) = delete;
	Game(const Game &) = delete;
	void operator=(Game &&) = delete;
	void operator=(const Game &) = delete;
	~Game();
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
	void put_string(const Point &position, TileRegion region, const char *string);
	void run_dialog(TextResourceId);
	void reset_dialog_state();
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
	void teleport_player(Map destination, const Point &position);
	void game_loop();

	DEFINE_GETTER_SETTER(options)
	DEFINE_GETTER_SETTER(options_initialized)
};

}
