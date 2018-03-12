#pragma once
#include "AudioData.h"
#include <SDL.h>

class AudioRenderer;

class AbstractAudioDevice{
public:
	AbstractAudioDevice() = default;
	virtual ~AbstractAudioDevice(){}
	virtual void set_renderer(AudioRenderer &) = 0;
	virtual void clear_renderer() = 0;
};

class AudioDevice : public AbstractAudioDevice{
	SDL_AudioDeviceID audio_device = 0;
	AudioRenderer *renderer = nullptr;
	StereoSampleFinal spillover_buffer[AudioFrame::length];
	size_t spillover_buffer_size = 0;
	void audio_callback(Uint8 *stream, int len);
	static void SDLCALL audio_callback(void *userdata, Uint8 *stream, int len){
		((AudioDevice *)userdata)->audio_callback(stream, len);
	}
public:
	AudioDevice();
	~AudioDevice();
	void set_renderer(AudioRenderer &) override;
	void clear_renderer() override;
};
