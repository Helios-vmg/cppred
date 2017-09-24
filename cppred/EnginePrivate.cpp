#include "EnginePrivate.h"
#include "CppRedEntryPoint.h"
#include <stdexcept>
#include <cassert>

Engine::Pimpl::Pimpl(Engine &engine):
		engine(&engine),
		prng(get_seed()),
		main_thread_id(std::this_thread::get_id()){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	
	this->initialize_window();
	this->initialize_video();
	this->initialize_audio();
}

Engine::Pimpl::~Pimpl(){
	SDL_Quit();
}

void Engine::Pimpl::initialize_window(){
	this->window = SDL_CreateWindow("POKE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Renderer::logical_screen_width * screen_scale, Renderer::logical_screen_height * screen_scale, 0);
	if (!this->window)
		throw std::runtime_error("Failed to initialize SDL window.");
}

void Engine::Pimpl::initialize_video(){
	this->renderer.reset(new Renderer(*this->engine, this->window));
}

void Engine::Pimpl::initialize_audio(){
#if 0
	SDL_AudioSpec desired, actual;
	memset(&desired, 0, sizeof(desired));
	desired.freq = 44100;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = AudioFrame::length;
	desired.callback = SdlProvider::audio_callback;
	desired.userdata = this;
	this->audio_device = SDL_OpenAudioDevice(nullptr, false, &desired, &actual, 0);
	if (!(actual == desired)){
		SDL_CloseAudioDevice(this->audio_device);
		this->audio_device = 0;
		return;
	}
	SDL_PauseAudioDevice(this->audio_device, 0);
#endif
}

void Engine::Pimpl::run(){
	if (std::this_thread::get_id() != this->main_thread_id)
		throw std::runtime_error("Engine::run() must be called from the main thread!");

	this->coroutine.reset(new coroutine_t([this](yielder_t &y){ this->coroutine_entry_point(y); }));
	auto yielder = this->yielder;
	this->yielder = nullptr;

	//Main loop.
	while (this->handle_events()){
		//Resume game code.
		std::swap(yielder, this->yielder);
		auto stop = !(*this->coroutine)();
		std::swap(yielder, this->yielder);
		if (stop)
			break;

		//Process audio.

		this->renderer->render();
	}

	this->coroutine.reset();
}

void Engine::Pimpl::coroutine_entry_point(yielder_t &yielder){
	this->yielder = &yielder;
	this->yield();
	CppRed::entry_point(*this->engine);
}

void Engine::Pimpl::yield(){
	if (std::this_thread::get_id() != this->main_thread_id)
		throw std::runtime_error("Engine::yield() must be called from the main thread!");
	if (!this->yielder)
		throw std::runtime_error("Engine::yield() must be called while the coroutine is active!");
	(*this->yielder)();
}

void Engine::Pimpl::wait(double s){
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

double Engine::Pimpl::get_clock(){
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

bool Engine::Pimpl::handle_events(){
	SDL_Event event;
	auto &state = this->input_state;
	bool button_down = false;
	bool button_up = false;
	while (SDL_PollEvent(&event)){
		switch (event.type){
			case SDL_QUIT:
				return false;
			case SDL_KEYDOWN:
				if (!event.key.repeat)
					handle_event<true>(state, event, button_down);
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
