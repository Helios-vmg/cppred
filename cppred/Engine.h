#pragma once
#include "utility.h"
#include "InputState.h"
#include "Renderer.h"
#include "HighResolutionClock.h"
#include "ScriptStore.h"
#include <thread>
#include <mutex>
#include <memory>

enum class PokemonVersion;
class XorShift128;
class Renderer;
class Console;
class AudioDevice;
class AudioScheduler;
class TwoWayMixer;
struct SDL_Window;
typedef struct SDL_Window SDL_Window;

namespace CppRed{
class AudioProgramInterface;
class Game;
}

#define Engine_USE_FIXED_CLOCK

class Engine{
#ifndef Engine_USE_FIXED_CLOCK
	HighResolutionClock base_clock;
	SteppingClock clock;
#else
	FixedClock clock;
#endif
	SDL_Window *window = nullptr;
	std::unique_ptr<AudioDevice> audio_device;
	std::unique_ptr<VideoDevice> video_device;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<CppRed::Game> game;
	XorShift128 prng;
	InputState input_state;
	std::unique_ptr<AudioScheduler> audio_scheduler;
	std::unique_ptr<Console> console;
	bool debug_mode = false;
	std::mutex exception_thrown_mutex;
	std::unique_ptr<std::string> exception_thrown;
	ScriptStore script_store;
	bool gamepad_disabled = false;
	TwoWayMixer *two_way_mixer = nullptr;

	void initialize_video();
	void initialize_audio();
	bool handle_events();
	bool update_console(PokemonVersion &version, CppRed::AudioProgramInterface &program);
	void check_exceptions();
public:
	Engine();
	~Engine();
	Engine(const Engine &) = delete;
	Engine(Engine &&other) = delete;
	void operator=(const Engine &) = delete;
	void operator=(Engine &&) = delete;
	void run();
	DEFINE_NON_CONST_GETTER(prng)
	Renderer &get_renderer(){
		return *this->renderer;
	}
	SteppingClock &get_stepping_clock(){
		return this->clock;
	}
	void execute_script(const CppRed::Scripts::script_parameters &parameter) const;
	ScriptStore::script_f get_script(const char *script_name) const;
	InputState get_input_state() const{
		return this->gamepad_disabled ? InputState() : this->input_state;
	}
	DEFINE_GETTER_SETTER(gamepad_disabled)
	TwoWayMixer &get_mixer(){
		return *this->two_way_mixer;
	}

	void go_to_debug();
	void restart();
	void throw_exception(const std::exception &e);
	static const int screen_scale;
	static const int dmg_clock_frequency = 1 << 22;
	static const int dmg_display_period = 70224;
	static const double logical_refresh_rate;
	static const double logical_refresh_period;
};
