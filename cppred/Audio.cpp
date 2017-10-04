#include "Audio.h"
#include "SoundGenerators.h"
#include "Engine.h"
#include "ActualAudioRenderer.h"
#include <cstring>
#include <algorithm>

bool operator==(const SDL_AudioSpec &a, const SDL_AudioSpec &b){
	return
		a.freq == b.freq &&
		a.format == b.format &&
		a.channels == b.channels &&
		a.samples == b.samples;
}

basic_StereoSample<std::int16_t> convert(const basic_StereoSample<intermediate_audio_type> &src){
#ifdef USE_FLOAT_AUDIO
	basic_StereoSample<std::int16_t> ret;
	ret.left = (std::int16_t)(src.left * int16_max);
	ret.right = (std::int16_t)(src.right * int16_max);
	return ret;
#else
	basic_StereoSample<std::int16_t> ret;
	ret.left = (std::int16_t)src.left;
	ret.right = (std::int16_t)src.right;
	return ret;
#endif
}

AudioRenderer::AudioRenderer(Engine &engine): engine(&engine){
	this->renderer.reset(new ActualRenderer);
	this->continue_running = false;
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

	this->timer_id = SDL_AddTimer(1, timer_callback, this);
}

AudioRenderer::~AudioRenderer(){
	if (this->thread){
		this->continue_running = false;
		this->thread->join();
		this->thread.reset();
	}
	if (this->timer_id)
		SDL_RemoveTimer(this->timer_id);
	if (this->audio_device){
		SDL_PauseAudioDevice(this->audio_device, 1);
		SDL_CloseAudioDevice(this->audio_device);
		this->audio_device = 0;
	}
}

void SDLCALL AudioRenderer::audio_callback(void *userdata, Uint8 *stream, int len){
	auto This = (AudioRenderer *)userdata;
	{
		std::lock_guard<std::mutex> lg(This->mutex);
		auto frame = This->get_current_frame();
		if (frame){
			if (frame->frame_no < This->next_frame){
				This->return_used_frame(frame);
			}else{
				This->next_frame = frame->frame_no + 1;
#ifndef BENCHMARKING
				auto n = std::min<size_t>(len, sizeof(frame->buffer));
				memcpy(stream, frame->buffer, n);
				if (len - n)
					memset(stream + n, 0, len - n);
#else
				memset(stream, 0, len);
#endif
				This->return_used_frame(frame);
				return;
			}
		}
	}
	memset(stream, 0, len);
}

Uint32 SDLCALL AudioRenderer::timer_callback(Uint32 interval, void *param){
	auto This = (AudioRenderer *)param;
	This->timer_event.signal();
	return interval;
}

AudioFrame *AudioRenderer::get_current_frame(){
	return this->renderer->publishing_frames.get_public_resource();
}

void AudioRenderer::return_used_frame(AudioFrame *frame){
	this->renderer->publishing_frames.return_resource(frame);
}

void AudioRenderer::start_audio_processing(AudioProgram &program){
	this->continue_running = true;
	this->thread.reset(new std::thread([this, &program](){ this->processor(program); }));
}

void AudioRenderer::processor(AudioProgram &program){
	while (this->continue_running){
		auto now = this->engine->get_clock();
		program.update(now, *this);
		this->renderer->update(now);
		this->timer_event.wait();
	}
}

void AudioRenderer::set_nr30(byte_t value){
	this->renderer->set_NR30(value);
}

void AudioRenderer::set_nr51(byte_t value){
	this->renderer->set_NR51(value);
}
