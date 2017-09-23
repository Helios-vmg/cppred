#pragma once
#include "CppRedConstants.h"

class CppRed;

class CppRedAudio{
	CppRed *parent;
public:
	CppRedAudio(CppRed &parent);
	void audio_play_sound(Sound){}
	void audio_update_music();
	void audio_apply_music_effects(unsigned channel){}
	void music_do_low_health_alert(){}
	void fade_out_audio();
	void play_sound(Sound sound);
};
