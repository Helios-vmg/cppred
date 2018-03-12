#include "AudioDevice.h"
#include "AudioData.h"
#include "AudioRenderer.h"
#include "utility.h"

bool operator==(const SDL_AudioSpec &a, const SDL_AudioSpec &b){
	return
		a.freq == b.freq &&
		a.format == b.format &&
		a.channels == b.channels;
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

void AudioDevice::audio_callback(Uint8 *stream, int len){
	if (!this->renderer){
		memset(stream, 0, len);
		return;
	}
	if (this->spillover_buffer_size){
		const auto s = sizeof(StereoSampleFinal);
		const auto size2 = (int)(this->spillover_buffer_size * s);
		if (len >= size2){
			memcpy(stream, this->spillover_buffer, size2);
			stream += size2;
			len -= size2;
			this->spillover_buffer_size = 0;
		}else{
			memcpy(stream, this->spillover_buffer, len);

			const auto m = (len + (s - 1)) / s * s;
			assert(m <= size2);
			if (m < size2){
				memmove(this->spillover_buffer, (byte_t *)this->spillover_buffer + m, size2 - m);
				this->spillover_buffer_size = (size2 - m) / s;
			}

			stream += len;
			len = 0;
		}
	}

	if (len)
		this->renderer->write_data_to_device(stream, len, this->spillover_buffer, this->spillover_buffer_size);
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

void AudioDevice::set_renderer(AudioRenderer &renderer){
	AudioLock al(this->audio_device);
	this->renderer = &renderer;
}

void AudioDevice::clear_renderer(){
	AudioLock al(this->audio_device);
	this->renderer = nullptr;
}
