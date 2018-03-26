#include "Engine.h"
#include "CppRed/AudioProgram.h"
#include "CppRed/Game.h"
#include "CppRed/PlayerCharacter.h"
#include "CppRed/World.h"
#include "AudioScheduler.h"
#include "AudioDevice.h"
#include "AudioRenderer.h"
#include "HeliosRenderer.h"
#include "Console.h"
#include <stdexcept>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <SDL.h>

const double Engine::logical_refresh_rate = (double)dmg_clock_frequency / dmg_display_period;
const double Engine::logical_refresh_period = (double)dmg_display_period / dmg_clock_frequency;
const int Engine::screen_scale = 4;

Engine::Engine():
#ifndef Engine_USE_FIXED_CLOCK
		clock(this->base_clock),
#endif
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
	this->video_device = Renderer::initialize_device(screen_scale);
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
//#define CPU_USAGE
#endif

void Engine::run(){
	PokemonVersion version = PokemonVersion::Red;
	bool continue_running = true;
#ifdef CPU_USAGE
	HighResolutionClock clock;
	double last = clock.get();
	double time_processing = 0;
	double logic_time = 0;
	double renderer_time = 0;
#endif
	while (continue_running){
		this->video_device->set_window_title(to_string(version));
		this->debug_mode = false;
		this->renderer.reset(new Renderer(*this->video_device));
		if (!this->console){
			this->console.reset(new Console(*this));
			global_console = this->console.get();
		}
		auto two_way_mixer = std::make_unique<TwoWayMixer>(*this->audio_device);
		this->two_way_mixer = two_way_mixer.get();
		two_way_mixer->set_renderers(std::make_unique<HeliosRenderer>(*two_way_mixer), std::make_unique<HeliosRenderer>(*two_way_mixer));
		auto interfacep = std::make_unique<CppRed::AudioProgramInterface>(two_way_mixer->get_low_priority_renderer(), two_way_mixer->get_high_priority_renderer(), version);
		auto &interface = *interfacep;
		this->audio_scheduler.reset(new AudioScheduler(*this, std::move(two_way_mixer), std::move(interfacep)));
		this->audio_scheduler->start();
		this->gamepad_disabled = false;
		this->game.reset(new CppRed::Game(*this, version, interface));

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

			if (!this->debug_mode)
				this->game->update();

#ifdef CPU_USAGE
			auto t1 = clock.get();
#endif

			this->renderer->render();
			this->console->render();
#ifdef CPU_USAGE
			auto t2 = clock.get();
			logic_time += t1 - t0;
			renderer_time += t2 - t1;
			time_processing += t2 - t0;
			if (t2 >= last + 1){
				std::stringstream temp;
				auto total = t2 - last;
				auto total_usage = time_processing / total * 100;
				auto current_logic_time = (int)(logic_time / time_processing * 100);
				auto current_renderer_time = (int)(renderer_time / time_processing * 100);
				temp << "Engine::run() CPU usage: " << std::setw(5) << std::setprecision(3) << total_usage << " % (logic: " << current_logic_time << " %, renderer: " << current_renderer_time << "%)\n";
				this->console->log_string(temp.str());
				last = t2;
				logic_time = 0;
				renderer_time = 0;
				time_processing = 0;
			}
#endif
			this->video_device->present();
		}

		this->game.reset();
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
			case SDL_MOUSEBUTTONDOWN:
				{
					auto &world = this->game->get_world();
					if (world.player_initialized()){
						Point location(event.button.x, event.button.y);
						location.x = location.x / (this->screen_scale * Renderer::tile_size * 2);
						location.y = location.y / (this->screen_scale * Renderer::tile_size * 2);
						location -= CppRed::PlayerCharacter::screen_block_offset;
						auto &pc = world.get_pc();
						location += pc.get_map_position();
						MapObjectInstance *objects[8];
						world.get_objects_at_location(objects, {pc.get_current_map(), location});
						std::stringstream stream;
						stream << "Clicked at " << location << '\n';
						for (auto object : objects){
							if (!object)
								break;
							stream << "Found a " << object->get_object().get_type_string() << " named " << object->get_object().get_name() << '\n';
						}
						Logger() << stream.str();
					}
					break;
				}
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

void Engine::go_to_debug(){
	this->debug_mode = true;
}

void Engine::throw_exception(const std::exception &e){
	LOCK_MUTEX(this->exception_thrown_mutex);
	this->exception_thrown = std::make_unique<std::string>(e.what());
}

void Engine::execute_script(const CppRed::Scripts::script_parameters &parameter) const{
	this->script_store.execute(parameter);
}

ScriptStore::script_f Engine::get_script(const char *script_name) const{
	return this->script_store.get_script(script_name);
}
