#include "AudioDevice.h"
#include "AudioData.h"
#include "utility.h"

bool operator==(const SDL_AudioSpec &a, const SDL_AudioSpec &b){
	return
		a.freq == b.freq &&
		a.format == b.format &&
		a.channels == b.channels &&
		a.samples == b.samples;
}

AudioDevice::AudioDevice(){
	SDL_AudioSpec desired, actual;
	memset(&desired, 0, sizeof(desired));
	desired.freq = sampling_frequency;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = AudioFrame::length;
	desired.callback = audio_callback;
	desired.userdata = this;
	this->audio_device = SDL_OpenAudioDevice(nullptr, false, &desired, &actual, 0);
	if (!(actual == desired)){
		SDL_CloseAudioDevice(this->audio_device);
		this->audio_device = 0;
		return;
	}
	SDL_PauseAudioDevice(this->audio_device, 0);
}

AudioDevice::~AudioDevice(){
	if (this->audio_device){
		SDL_PauseAudioDevice(this->audio_device, 1);
		SDL_CloseAudioDevice(this->audio_device);
		this->audio_device = 0;
	}
}

void SDLCALL AudioDevice::audio_callback(void *userdata, Uint8 *stream, int len){
	auto This = (AudioDevice *)userdata;
	if (!This->renderer){
		memset(stream, 0, len);
		return;
	}
	This->renderer->write_data_to_device(stream, len);
}

class AudioLock{
	SDL_AudioDeviceID dev;
public:
	AudioLock(SDL_AudioDeviceID dev): dev(dev){
		SDL_LockAudioDevice(this->dev);
	}
	~AudioLock(){
		SDL_UnlockAudioDevice(this->dev);
	}
};

void AudioDevice::set_renderer(AudioRenderer2 &renderer){
	AudioLock al(this->audio_device);
	this->renderer = &renderer;
}

void AudioDevice::clear_renderer(){
	AudioLock al(this->audio_device);
	this->renderer = nullptr;
}
