#include "Engine.h"
#include "CppRed/EntryPoint.h"
#include "CppRed/AudioProgram.h"
#include "AudioScheduler.h"
#include "AudioDevice.h"
#include "AudioRenderer.h"
#include "HeliosRenderer.h"
#include "Console.h"
#include <stdexcept>
#include <cassert>

const double Engine::logical_refresh_rate = (double)dmg_clock_frequency / dmg_display_period;
const double Engine::logical_refresh_period = (double)dmg_display_period / dmg_clock_frequency;

Engine::Engine():
		clock(this->base_clock),
		prng(get_seed()){
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

static const char *to_string(PokemonVersion version){
	switch (version){
		case PokemonVersion::Red:
			return "Pok\xC3\xA9mon Red";
		case PokemonVersion::Blue:
			return "Pok\xC3\xA9mon Blue";
		default:
			return "Pok\xC3\xA9mon ?";
	}
}

#ifdef interface
#undef interface
#endif

#if defined CPPRED_TESTING
#define CPU_USAGE
#endif

void Engine::run(){
	PokemonVersion version = PokemonVersion::Red;
	bool continue_running = true;
#ifdef CPU_USAGE
		HighResolutionClock clock;
		double last = clock.get();
		double time_processing = 0;
#endif
	while (continue_running){
		this->video_device->set_window_title(to_string(version));
		this->debug_mode = false;
		this->renderer.reset(new Renderer(*this->video_device));
		if (!this->console)
			this->console.reset(new Console(*this));
		auto two_way_mixer = std::make_unique<TwoWayMixer>(*this->audio_device);
		two_way_mixer->set_renderers(std::make_unique<HeliosRenderer>(*two_way_mixer), std::make_unique<HeliosRenderer>(*two_way_mixer));
		auto interfacep = std::make_unique<CppRed::AudioProgramInterface>(two_way_mixer->get_low_priority_renderer(), two_way_mixer->get_high_priority_renderer(), version);
		auto &interface = *interfacep;
		this->audio_scheduler.reset(new AudioScheduler(*this, std::move(two_way_mixer), std::move(interfacep)));
		this->audio_scheduler->start();
		this->coroutine.reset(new Coroutine(
			"Main coroutine",
			this->clock,
			[this, version, &interface](Coroutine &){ this->coroutine_entry_point(version, interface); })
		);

		//Main loop.
		while (true){
#ifdef CPU_USAGE
			auto t0 = clock.get();
#endif
			this->clock.step();
			continue_running &= this->handle_events();
			if (!continue_running)
				break;
			if (!this->update_console(version, interface))
				break;
			this->check_exceptions();

			if (!this->debug_mode){
				//Resume game code.
				this->coroutine->get_clock().step();
				continue_running &= this->coroutine->resume();
			}

			this->renderer->render();
			this->console->render();
#ifdef CPU_USAGE
			auto t1 = clock.get();
			time_processing += t1 - t0;
			if (t1 >= last + 1){
				std::cout << "Engine::run() CPU usage: " << time_processing / (t1 - last) * 100 << " %\n";
				last = t1;
				time_processing = 0;
			}
#endif
			this->video_device->present();
		}

		this->coroutine.reset();
		this->audio_scheduler.reset();
	}
}

void Engine::check_exceptions(){
	LOCK_MUTEX(this->exception_thrown_mutex);
	if (this->exception_thrown)
		throw std::runtime_error(*this->exception_thrown);
}

bool Engine::update_console(PokemonVersion &version, CppRed::AudioProgramInterface &program){
	while (true){
		auto console_request = this->console->update();
		if (!console_request)
			return true;
		switch (console_request->request_id){
			case ConsoleRequestId::GetAudioProgram:
				console_request->audio_program = &program;
				break;
			case ConsoleRequestId::Restart:
				return false;
			case ConsoleRequestId::FlipVersion:
				version = version == PokemonVersion::Red ? PokemonVersion::Blue : PokemonVersion::Red;
				break;
			case ConsoleRequestId::GetVersion:
				console_request->version = version;
				break;
			default:
				return true;
		}
	}
	return true;
}


void Engine::coroutine_entry_point(PokemonVersion version, CppRed::AudioProgramInterface &program){
	this->yield();
	CppRed::Scripts::entry_point(*this, version, program);
}

void Engine::yield(){
	this->coroutine->yield();
}

void Engine::wait(double s){
	this->coroutine->wait(s);
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

void Engine::set_on_yield(std::function<void()> &&callback){
	this->coroutine->set_on_yield(std::move(callback));
}

void Engine::go_to_debug(){
	this->debug_mode = true;
}

void Engine::throw_exception(const std::exception &e){
	LOCK_MUTEX(this->exception_thrown_mutex);
	this->exception_thrown = std::make_unique<std::string>(e.what());
}
