#pragma once
#include "utility.h"
#include "InputState.h"
#include "Renderer.h"
#include "Audio.h"
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

class XorShift128;
class Renderer;
class Console;

class Engine{
	HighResolutionClock clock;
	SDL_Window *window = nullptr;
	std::unique_ptr<Renderer> renderer;
	XorShift128 prng;
	typedef boost::coroutines2::asymmetric_coroutine<void>::pull_type coroutine_t;
	typedef boost::coroutines2::asymmetric_coroutine<void>::push_type yielder_t;
	std::unique_ptr<coroutine_t> coroutine;
	yielder_t *yielder = nullptr;
	std::thread::id main_thread_id;
	double wait_remainder = 0;
	InputState input_state;
	std::function<void()> on_yield;
	std::unique_ptr<AudioSystem> audio;
	std::unique_ptr<Console> console;
	bool debug_mode = false;

	void initialize_window();
	void initialize_video();
	void initialize_audio();
	void coroutine_entry_point(yielder_t &);
	bool handle_events();
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
	AudioSystem &get_audio(){
		return *this->audio;
	}
	void yield();
	void wait(double seconds);
	//Note: Doesn't actually wait a specific number of frames. It multiplies
	//the argument by a time constant and waits that much time instead.
	void wait_frames(int frames);
	void wait_exactly_one_frame(){
		this->yield();
	}
	double get_clock();
	void set_on_yield(std::function<void()> &&);
	DEFINE_GETTER(input_state)

	void play_sound(AudioResourceId sound){
		this->audio->play_sound(sound);
	}
	void go_to_debug();
	static const int screen_scale = 4;
	static const int dmg_clock_frequency = 1 << 22;
	static const int dmg_display_period = 70224;
	static const double logical_refresh_rate;
	static const double logical_refresh_period;
};
