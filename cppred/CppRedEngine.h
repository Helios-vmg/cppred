#pragma once
#include "Engine.h"
#include "../CodeGeneration/output/sounds.h"
#include "utility.h"
#include "CppRedData.h"
#include "CppRedSavableData.h"
#include "CppRedTextResources.h"
#include <initializer_list>
#include <string>

#ifdef max
#undef max
#endif

class CppRedEngine{
	Engine *engine;
	TextStore store;
	InputState jls_last_state;
	double jls_timeout = std::numeric_limits<double>::max();
	GameOptions options;
	bool options_initialized = false;

public:
	CppRedEngine(Engine &engine);
	void clear_screen();
	Engine &get_engine(){
		return *this->engine;
	}
	void play_sound(SoundId);
	void play_cry(SpeciesId);
	void fade_out_to_white();
	void palette_whiteout();
	bool check_for_user_interruption(double timeout = 0, InputState * = nullptr);
	bool check_for_user_interruption(InputState &is){
		return this->check_for_user_interruption(0, &is);
	}
	InputState joypad_low_sensitivity();
	void wait_for_sound_to_finish();
	typedef decltype(SavableData::load("")) load_save_t;
	load_save_t load_save();
	void draw_box(const Point &corner, const Point &size, TileRegion);
	int handle_standard_menu(
		TileRegion region,
		const Point &position,
		const std::vector<std::string> &items,
		int minimum_width = 0,
		bool ignore_b = false
	);
	void put_string(const Point &position, TileRegion region, const char *string);

	DEFINE_GETTER_SETTER(options)
	DEFINE_GETTER_SETTER(options_initialized)
};
