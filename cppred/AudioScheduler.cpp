#include "AudioScheduler.h"
#include "Engine.h"
#include "AudioRenderer.h"
#include "CppRed/AudioProgram.h"
#include "../CodeGeneration/output/audio.h"

AudioScheduler::AudioScheduler(
			Engine &engine,
			std::unique_ptr<AudioRenderer> &&renderer,
			std::unique_ptr<CppRed::AudioProgramInterface> &&program_interface
		): engine(&engine), renderer(std::move(renderer)), program_interface(std::move(program_interface)){
	this->continue_running = false;
	this->timer_id = SDL_AddTimer(1, timer_callback, this);
}

AudioScheduler::~AudioScheduler(){
	this->stop();
	if (this->timer_id)
		SDL_RemoveTimer(this->timer_id);
}

void AudioScheduler::start(){
	if (this->thread.joinable())
		return;
	this->continue_running = true;
	this->thread = std::thread([this](){ this->processor(); });
}

#ifdef CPPRED_TESTING
#define CPU_USAGE
#endif

void AudioScheduler::processor(){
	try{
		this->renderer->start();
#ifdef CPU_USAGE
		HighResolutionClock clock;
		double last = clock.get();
		double time_processing = 0;
#endif
		while (this->continue_running){
#ifdef CPU_USAGE
			auto t0 = clock.get();
#endif
			auto now = this->engine->get_clock();
			this->program_interface->update(now);
			this->renderer->update(now);
#ifdef CPU_USAGE
			auto t1 = clock.get();
			time_processing += t1 - t0;
			if (t1 >= last + 1){
				std::cout << "AudioScheduler::processor() CPU usage: " << time_processing / (t1 - last) * 100 << " %\n";
				last = t1;
				time_processing = 0;
			}
#endif
			//Delay for ~1 ms. Experimentation shows that, at least on Windows, the
			//actual wait can last up to a few ms.
			this->timer_event.wait();
		}
	}catch (std::exception &e){
		this->engine->throw_exception(e);
	}
}

void AudioScheduler::stop(){
	if (this->thread.joinable()){
		this->continue_running = false;
		this->thread.join();
	}
}

Uint32 SDLCALL AudioScheduler::timer_callback(Uint32 interval, void *param){
	auto This = (AudioScheduler *)param;
	This->timer_event.signal();
	return interval;
}
