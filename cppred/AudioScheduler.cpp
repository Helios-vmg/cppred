#include "AudioScheduler.h"
#include "Engine.h"
#include "AudioRenderer.h"
#include "CppRed/AudioProgram.h"
#include "../CodeGeneration/output/audio.h"

AudioScheduler::AudioScheduler(Engine &engine, std::unique_ptr<AudioRenderer> &&renderer, std::unique_ptr<CppRed::AudioProgram> &&program): engine(&engine){
	this->renderer = std::move(renderer);
	this->program = std::move(program);
	this->continue_running = false;
	this->timer_id = SDL_AddTimer(1, timer_callback, this);
}

AudioScheduler::~AudioScheduler(){
	this->stop();
	if (this->timer_id)
		SDL_RemoveTimer(this->timer_id);
}

void AudioScheduler::start(){
	if (this->thread)
		return;
	this->continue_running = true;
	this->thread.reset(new std::thread([this](){ this->processor(); }));
}

void AudioScheduler::processor(){
	try{
		this->renderer->set_NR52(0xFF);
		this->renderer->set_NR50(0x77);
		while (this->continue_running){
			auto now = this->engine->get_clock();
			this->program->update(now);
			this->renderer->update(now);
			//Delay for ~1 ms. Experimentation shows that, at least on Windows, the
			//actual wait can last up to a few ms.
			this->timer_event.wait();
		}
	}catch (std::exception &e){
		this->engine->throw_exception(e);
	}
}

void AudioScheduler::stop(){
	if (this->thread){
		this->continue_running = false;
		this->thread->join();
		this->thread.reset();
	}
}

Uint32 SDLCALL AudioScheduler::timer_callback(Uint32 interval, void *param){
	auto This = (AudioScheduler *)param;
	This->timer_event.signal();
	return interval;
}
