#include "Audio.h"
#include "SoundGenerators.h"
#include "Engine.h"
#include "ActualAudioRenderer.h"
#include "utility.h"
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
	this->AudioRenderer::stop_audio_processing();
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
		LOCK_MUTEX(This->mutex);
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
	if (this->thread)
		this->stop_audio_processing();
	this->continue_running = true;
	this->thread.reset(new std::thread([this, &program](){ this->processor(program); }));
}

void AudioRenderer::stop_audio_processing(){
	if (this->thread){
		this->continue_running = false;
		this->thread->join();
		this->thread.reset();
	}
}

void AudioRenderer::process_queue(AudioProgram &program){
	decltype(this->request_queue) queue;
	{
		LOCK_MUTEX(this->queue_mutex);
		queue = std::move(this->request_queue);
	}
	for (auto request : queue)
		this->play_sound_internal(program, request);
}

void AudioRenderer::processor(AudioProgram &program){
	this->renderer->set_NR52(0xFF);
	while (this->continue_running){
		auto now = this->engine->get_clock();
		this->process_queue(program);
		program.update(now);
		this->renderer->update(now);
		//Delay for ~1 ms. Experimentation shows that, at least on Windows, the
		//actual wait can last up to a few ms.
		this->timer_event.wait();
	}
}

void AudioRenderer::set_NR10(byte_t value){
	this->renderer->square1.set_register0(value);
}

void AudioRenderer::set_NR11(byte_t value){
	this->renderer->square1.set_register1(value);
}

void AudioRenderer::set_NR12(byte_t value){
	this->renderer->square1.set_register2(value);
}

void AudioRenderer::set_NR13(byte_t value){
	this->renderer->square1.set_register3(value);
}

void AudioRenderer::set_NR14(byte_t value){
	this->renderer->square1.set_register4(value);
}

void AudioRenderer::set_NR21(byte_t value){
	this->renderer->square2.set_register1(value);
}

void AudioRenderer::set_NR22(byte_t value){
	this->renderer->square2.set_register2(value);
}

void AudioRenderer::set_NR23(byte_t value){
	this->renderer->square2.set_register3(value);
}

void AudioRenderer::set_NR24(byte_t value){
	this->renderer->square2.set_register4(value);
}

void AudioRenderer::set_NR30(byte_t value){
	this->renderer->wave.set_register0(value);
}

void AudioRenderer::set_NR31(byte_t value){
	this->renderer->wave.set_register1(value);
}

void AudioRenderer::set_NR32(byte_t value){
	this->renderer->wave.set_register2(value);
}

void AudioRenderer::set_NR33(byte_t value){
	this->renderer->wave.set_register3(value);
}

void AudioRenderer::set_NR34(byte_t value){
	this->renderer->wave.set_register4(value);
}

void AudioRenderer::set_NR41(byte_t value){
	this->renderer->noise.set_register1(value);
}

void AudioRenderer::set_NR42(byte_t value){
	this->renderer->noise.set_register2(value);
}

void AudioRenderer::set_NR43(byte_t value){
	this->renderer->noise.set_register3(value);
}

void AudioRenderer::set_NR44(byte_t value){
	this->renderer->noise.set_register4(value);
}

void AudioRenderer::set_NR50(byte_t value){
	this->renderer->set_NR50(value);
}

void AudioRenderer::set_NR51(byte_t value){
	this->renderer->set_NR51(value);
}

void AudioRenderer::set_NR52(byte_t value){
	this->renderer->set_NR52(value);
}

byte_t AudioRenderer::get_NR11(){
	return this->renderer->square1.get_register1();
}

byte_t AudioRenderer::get_NR12(){
	return this->renderer->square1.get_register2();
}

byte_t AudioRenderer::get_NR13(){
	return this->renderer->square1.get_register3();
}

byte_t AudioRenderer::get_NR14(){
	return this->renderer->square1.get_register4();
}

byte_t AudioRenderer::get_NR21(){
	return this->renderer->square2.get_register1();
}

byte_t AudioRenderer::get_NR22(){
	return this->renderer->square2.get_register2();
}

byte_t AudioRenderer::get_NR23(){
	return this->renderer->square2.get_register3();
}

byte_t AudioRenderer::get_NR24(){
	return this->renderer->square2.get_register4();
}

byte_t AudioRenderer::get_NR31(){
	return this->renderer->wave.get_register1();
}

byte_t AudioRenderer::get_NR32(){
	return this->renderer->wave.get_register2();
}

byte_t AudioRenderer::get_NR33(){
	return this->renderer->wave.get_register3();
}

byte_t AudioRenderer::get_NR34(){
	return this->renderer->wave.get_register4();
}

byte_t AudioRenderer::get_NR41(){
	return this->renderer->noise.get_register1();
}

byte_t AudioRenderer::get_NR42(){
	return this->renderer->noise.get_register2();
}

byte_t AudioRenderer::get_NR43(){
	return this->renderer->noise.get_register3();
}

byte_t AudioRenderer::get_NR44(){
	return this->renderer->noise.get_register4();
}

byte_t AudioRenderer::get_NR50(){
	return this->renderer->get_NR50();
}

byte_t AudioRenderer::get_NR51(){
	return this->renderer->get_NR51();
}

void AudioRenderer::copy_voluntary_wave(const void *buffer){
	for (int i = 16; i--;)
		this->renderer->wave.set_wave_table(i, ((const byte *)buffer)[i]);
}

void AudioRenderer::play_sound(AudioResourceId sound){
	LOCK_MUTEX(this->queue_mutex);
	this->request_queue.push_back(sound);
}

void AudioRenderer::play_sound_internal(AudioProgram &program, AudioResourceId sound){
	if (this->new_sound_id != AudioResourceId::None)
		for (int i = 4; i < 8; i++)
			program.clear_channel(i);
	if (this->fade_out_control){
		if (this->new_sound_id == AudioResourceId::None)
			return;
		this->new_sound_id = AudioResourceId::None;
		if (this->last_music_sound_id == AudioResourceId::Stop){
			this->after_fade_out_play_this = this->last_music_sound_id = sound;
			this->fade_out_counter = this->fade_out_counter_reload_value = this->fade_out_control;
			return;
		}
		this->fade_out_control = 0;
	}
	this->new_sound_id = AudioResourceId::None;
	program.play_sound(sound);
}
