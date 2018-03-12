#pragma once
#include "utility.h"
#include "InputState.h"
#include "Renderer.h"
#include "HighResolutionClock.h"
#include <SDL.h>
#include <boost/coroutine2/all.hpp>
#include <thread>
#include <memory>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

enum class PokemonVersion;
class XorShift128;
class Renderer;
class Console;
class AudioDevice;
class AudioScheduler;

namespace CppRed{
class AudioProgramInterface;
}

class Engine{
	HighResolutionClock clock;
	SDL_Window *window = nullptr;
	std::unique_ptr<AudioDevice> audio_device;
	std::unique_ptr<VideoDevice> video_device;
	std::unique_ptr<Renderer> renderer;
	XorShift128 prng;
	std::unique_ptr<Coroutine> coroutine;
	InputState input_state;
	std::unique_ptr<AudioScheduler> audio_scheduler;
	std::unique_ptr<Console> console;
	bool debug_mode = false;
	std::mutex exception_thrown_mutex;
	std::unique_ptr<std::string> exception_thrown;

	void initialize_video();
	void initialize_audio();
	void coroutine_entry_point(PokemonVersion, CppRed::AudioProgramInterface &);
	bool handle_events();
	bool update_console(PokemonVersion &version, CppRed::AudioProgramInterface &program);
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
	void yield();
	void wait(double seconds);
	//Note: Doesn't actually wait a specific number of frames. It multiplies
	//the argument by a time constant and waits that much time instead.
	//void wait_frames(int frames);
	void wait_exactly_one_frame(){
		this->yield();
	}
	double get_clock();
	void set_on_yield(std::function<void()> &&);
	DEFINE_GETTER(input_state)

	void go_to_debug();
	void restart();
	void throw_exception(const std::exception &e);
	static const int screen_scale = 4;
	static const int dmg_clock_frequency = 1 << 22;
	static const int dmg_display_period = 70224;
	static const double logical_refresh_rate;
	static const double logical_refresh_period;
};
