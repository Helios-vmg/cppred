#include "Audio.h"
#include "SoundGenerators.h"
#include "Engine.h"
#include "AudioRenderer.h"
#include "utility.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>

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

AudioSystem::AudioSystem(Engine &engine): engine(&engine){
#ifdef AudioRenderer_RECORD_AUDIO_REGISTER_WRITES
	this->audio_recording.open("audio_output.txt");
#endif
	this->renderer.reset(new HeliosRenderer);
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

AudioSystem::~AudioSystem(){
	this->AudioSystem::stop_audio_processing();
	if (this->timer_id)
		SDL_RemoveTimer(this->timer_id);
	if (this->audio_device){
		SDL_PauseAudioDevice(this->audio_device, 1);
		SDL_CloseAudioDevice(this->audio_device);
		this->audio_device = 0;
	}
}

void SDLCALL AudioSystem::audio_callback(void *userdata, Uint8 *stream, int len){
	auto This = (AudioSystem *)userdata;
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

Uint32 SDLCALL AudioSystem::timer_callback(Uint32 interval, void *param){
	auto This = (AudioSystem *)param;
	This->timer_event.signal();
	return interval;
}

AudioFrame *AudioSystem::get_current_frame(){
	return this->renderer->get_current_frame();
}

void AudioSystem::return_used_frame(AudioFrame *frame){
	this->renderer->return_used_frame(frame);
}

void AudioSystem::start_audio_processing(AudioProgram &program){
	if (this->thread)
		this->stop_audio_processing();
	this->continue_running = true;
	this->thread.reset(new std::thread([this, &program](){ this->processor(program); }));
}

void AudioSystem::stop_audio_processing(){
	if (this->thread){
		this->continue_running = false;
		this->thread->join();
		this->thread.reset();
	}
}

void AudioSystem::process_queue(AudioProgram &program){
	decltype(this->request_queue) queue;
	{
		LOCK_MUTEX(this->queue_mutex);
		queue = std::move(this->request_queue);
	}
	for (auto request : queue)
		this->play_sound_internal(program, request);
}

void AudioSystem::processor(AudioProgram &program){
	this->renderer->set_NR52(0xFF);
	this->renderer->set_NR50(0x77);
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

void AudioSystem::play_sound(AudioResourceId sound){
	LOCK_MUTEX(this->queue_mutex);
	this->request_queue.push_back(sound);
}

void AudioSystem::play_sound_internal(AudioProgram &program, AudioResourceId sound){
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

//-----------------------------------------------------------------------------
//Forwarders:

#ifdef AudioRenderer_RECORD_AUDIO_REGISTER_WRITES
#define OUTPUT_AUDIO(x) { \
	std::stringstream stream; \
	stream << "NR" #x " = " << std::hex << std::setw(2) << std::setfill('0') << (int)value; \
	auto s = stream.str(); \
	std::cout << s << std::endl; \
	this->audio_recording << s << std::endl; \
}
#else
#define OUTPUT_AUDIO(x)
#endif

#define DEFINE_FORWARDING_REGISTER_SET(reg)  \
void AudioSystem::set_NR##reg(byte_t value){ \
	OUTPUT_AUDIO(reg);                       \
	this->renderer->set_NR##reg(value);      \
}

#define DEFINE_FORWARDING_REGISTER_GET(reg)  \
byte_t AudioSystem::get_NR##reg(){           \
	return this->renderer->get_NR##reg();    \
}

DEFINE_FORWARDING_REGISTER_SET(10)
DEFINE_FORWARDING_REGISTER_SET(11)
DEFINE_FORWARDING_REGISTER_SET(12)
DEFINE_FORWARDING_REGISTER_SET(13)
DEFINE_FORWARDING_REGISTER_SET(14)

DEFINE_FORWARDING_REGISTER_SET(21)
DEFINE_FORWARDING_REGISTER_SET(22)
DEFINE_FORWARDING_REGISTER_SET(23)
DEFINE_FORWARDING_REGISTER_SET(24)

DEFINE_FORWARDING_REGISTER_SET(30)
DEFINE_FORWARDING_REGISTER_SET(31)
DEFINE_FORWARDING_REGISTER_SET(32)
DEFINE_FORWARDING_REGISTER_SET(33)
DEFINE_FORWARDING_REGISTER_SET(34)

DEFINE_FORWARDING_REGISTER_SET(41)
DEFINE_FORWARDING_REGISTER_SET(42)
DEFINE_FORWARDING_REGISTER_SET(43)
DEFINE_FORWARDING_REGISTER_SET(44)

DEFINE_FORWARDING_REGISTER_SET(50)
DEFINE_FORWARDING_REGISTER_SET(51)
DEFINE_FORWARDING_REGISTER_SET(52)

DEFINE_FORWARDING_REGISTER_GET(11)
DEFINE_FORWARDING_REGISTER_GET(12)
DEFINE_FORWARDING_REGISTER_GET(13)
DEFINE_FORWARDING_REGISTER_GET(14)

DEFINE_FORWARDING_REGISTER_GET(21)
DEFINE_FORWARDING_REGISTER_GET(22)
DEFINE_FORWARDING_REGISTER_GET(23)
DEFINE_FORWARDING_REGISTER_GET(24)

DEFINE_FORWARDING_REGISTER_GET(31)
DEFINE_FORWARDING_REGISTER_GET(32)
DEFINE_FORWARDING_REGISTER_GET(33)
DEFINE_FORWARDING_REGISTER_GET(34)

DEFINE_FORWARDING_REGISTER_GET(41)
DEFINE_FORWARDING_REGISTER_GET(42)
DEFINE_FORWARDING_REGISTER_GET(43)
DEFINE_FORWARDING_REGISTER_GET(44)

DEFINE_FORWARDING_REGISTER_GET(50)
DEFINE_FORWARDING_REGISTER_GET(51)

void AudioSystem::copy_voluntary_wave(const void *buffer){
#ifdef AudioRenderer_RECORD_AUDIO_REGISTER_WRITES
	for (int i = 0; i < 16; i++){
		std::stringstream stream;
		stream << "WAVE[" << std::hex << std::setw(2) << std::setfill('0') << i << "] = " << std::setw(2) << std::setfill('0') << (int)((const byte *)buffer)[i];
		auto s = stream.str();
		std::cout << s << std::endl;
		this->audio_recording << s << std::endl;
	}
#endif
	this->renderer->copy_voluntary_wave(buffer);
}
