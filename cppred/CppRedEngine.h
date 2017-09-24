#pragma once
#include "Engine.h"
#include "../CodeGeneration/output/sounds.h"
#include "utility.h"

enum class JlsMode{
	Normal,
	Delayed,
	HoldingRestricts,
};

class CppRedEngine{
	Engine *engine;
	JlsMode jls_mode = JlsMode::Normal;
	InputState jls_last_state;
	double jls_timeout = std::numeric_limits<double>::max();
public:
	CppRedEngine(Engine &engine);
	void clear_screen();
	Engine &get_engine(){
		return *this->engine;
	}
	void play_sound(SoundId);
	void fade_out_to_white();
	bool check_for_user_interruption(double timeout = 0, InputState * = nullptr);
	bool check_for_user_interruption(InputState &is){
		return this->check_for_user_interruption(0, &is);
	}
	InputState joypad_low_sensitivity();
	
	DEFINE_GETTER_SETTER(jls_mode)
};
