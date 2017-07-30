#pragma once
#include "CppRedConstants.h"

class CppRed;

class CppRedAudio{
	CppRed *parent;
public:
	CppRedAudio(CppRed &parent);
	void audio1_play_sound(Sound);
	void audio2_play_sound(Sound);
	void audio3_play_sound(Sound);
};
