#include "Engine.h"
#include "CppRedEntryPoint.h"
#include "CppRedAudioProgram.h"
#include "AudioScheduler.h"
#include "AudioDevice.h"
#include "HeliosRenderer.h"
#include "Console.h"
#include <stdexcept>
#include <cassert>

const double Engine::logical_refresh_rate = (double)dmg_clock_frequency / dmg_display_period;
const double Engine::logical_refresh_period = (double)dmg_display_period / dmg_clock_frequency;

Engine::Engine():
		prng(get_seed()),
		main_thread_id(std::this_thread::get_id()){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

	this->initialize_video();
	this->initialize_audio();
}

Engine::~Engine(){
	this->audio_scheduler.reset();
	SDL_Quit();
}

void Engine::initialize_video(){
	this->video_device = Renderer::initialize_device(4);
}

void Engine::initialize_audio(){
	this->audio_device.reset(new AudioDevice);
}

void Engine::run(){
	if (std::this_thread::get_id() != this->main_thread_id)
		throw std::runtime_error("Engine::run() must be called from the main thread!");

	PokemonVersion version = PokemonVersion::Red;
	bool continue_running = true;
	while (continue_running){
		this->wait_remainder = 0;
		this->restart_requested = false;
		this->debug_mode = false;
		this->renderer.reset(new Renderer(*this->video_device));
		if (!this->console)
			this->console.reset(new Console(*this));
		auto audio_renderer = std::make_unique<HeliosRenderer>(*this->audio_device);
		auto programp = std::make_unique<CppRedAudioProgram>(*audio_renderer, version);
		auto &program = *programp;
		this->audio_scheduler.reset(new AudioScheduler(*this, std::move(audio_renderer), std::move(programp)));
		this->audio_scheduler->start();
		this->coroutine.reset(new coroutine_t([this, version, &program](yielder_t &y){ this->coroutine_entry_point(y, version, program); }));
		auto yielder = this->yielder;
		this->yielder = nullptr;

		//Main loop.
		while ((continue_running &= this->handle_events()) && !this->restart_requested){
			if (!this->debug_mode){
				//Resume game code.
				std::swap(yielder, this->yielder);
				continue_running &= !!(*this->coroutine)();
				std::swap(yielder, this->yielder);
			}

			auto console_request = this->console->update();
			if (console_request){
				switch (console_request->request_id){
					case ConsoleRequestId::GetAudioProgram:
						console_request->audio_program = &program;
						break;
					default:
						break;
				}
			}

			this->renderer->render();
			this->console->render();
			this->video_device->present();
		}

		this->on_yield = decltype(this->on_yield)();
		this->coroutine.reset();
		this->audio_scheduler.reset();
	}
}

void Engine::coroutine_entry_point(yielder_t &yielder, PokemonVersion version, CppRedAudioProgram &program){
	this->yielder = &yielder;
	this->yield();
	CppRedScripts::entry_point(*this, version, program);
}

void Engine::yield(){
	if (std::this_thread::get_id() != this->main_thread_id)
		throw std::runtime_error("Engine::yield() must be called from the main thread!");
	if (!this->yielder)
		throw std::runtime_error("Engine::yield() must be called while the coroutine is active!");
	(*this->yielder)();
	if (this->on_yield)
		this->on_yield();
}

void Engine::wait(double s){
	auto target = this->get_clock() + s + this->wait_remainder;
	while (true){
		this->yield();
		auto now = this->get_clock();
		if (now >= target){
			this->wait_remainder = target - now;
			return;
		}
	}
}

double Engine::get_clock(){
	return this->clock.get();
}

template <bool DOWN>
static void handle_event(InputState &state, SDL_Event &event, bool &flag){
	switch (event.key.keysym.sym){
		case SDLK_UP:
			flag = true;
			state.set_up(DOWN);
			break;
		case SDLK_DOWN:
			flag = true;
			state.set_down(DOWN);
			break;
		case SDLK_LEFT:
			flag = true;
			state.set_left(DOWN);
			break;
		case SDLK_RIGHT:
			flag = true;
			state.set_right(DOWN);
			break;
		case SDLK_z:
			flag = true;
			state.set_a(DOWN);
			break;
		case SDLK_x:
			flag = true;
			state.set_b(DOWN);
			break;
		case SDLK_a:
			flag = true;
			state.set_start(DOWN);
			break;
		case SDLK_s:
			flag = true;
			state.set_select(DOWN);
			break;
	}
}

bool Engine::handle_events(){
	SDL_Event event;
	auto &state = this->input_state;
	bool button_down = false;
	bool button_up = false;
	while (SDL_PollEvent(&event)){
		if (this->console->handle_event(event))
			continue;
		switch (event.type){
			case SDL_QUIT:
				return false;
			case SDL_KEYDOWN:
				if (!event.key.repeat){
					switch (event.key.keysym.sym){
						case SDLK_ESCAPE:
							this->console->toggle_visible();
							break;
						default:
							handle_event<true>(state, event, button_down);
					}
				}
				break;
			case SDL_KEYUP:
				if (!event.key.repeat)
					handle_event<false>(state, event, button_up);
				break;
			default:
				break;
		}
	}
	const byte_t mask = InputState::mask_a | InputState::mask_b | InputState::mask_select | InputState::mask_start;
	if (button_down && (state.get_value() & mask) == mask){
		//Do soft reset
	}
	return true;
}

void Engine::wait_frames(int frames){
	this->wait(frames * logical_refresh_period);
}

void Engine::set_on_yield(std::function<void()> &&callback){
	this->on_yield = std::move(callback);
}

void Engine::go_to_debug(){
	this->debug_mode = true;
}

void Engine::restart(){
	this->restart_requested = true;
}
