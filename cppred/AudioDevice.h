#pragma once
#include "AudioRenderer.h"
#include <SDL.h>

class AudioDevice{
	SDL_AudioDeviceID audio_device = 0;
	AudioRenderer *renderer = nullptr;
	static void SDLCALL audio_callback(void *userdata, Uint8 *stream, int len);
public:
	AudioDevice();
	~AudioDevice();
	void set_renderer(AudioRenderer &);
	void clear_renderer();
};
