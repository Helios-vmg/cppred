#pragma once
#include "Engine.h"
#include "Renderer.h"
#include "HighResolutionClock.h"
#include "utility.h"
#include <SDL.h>
#include <boost/coroutine/coroutine.hpp>
#include <thread>

class Engine::Pimpl{
	Engine *engine;
	HighResolutionClock clock;
	SDL_Window *window = nullptr;
	std::unique_ptr<Renderer> renderer;
	XorShift128 prng;
	typedef boost::coroutines::asymmetric_coroutine<void>::pull_type coroutine_t;
	typedef boost::coroutines::asymmetric_coroutine<void>::push_type yielder_t;
	std::unique_ptr<coroutine_t> coroutine;
	yielder_t *yielder = nullptr;
	std::thread::id main_thread_id;
	double wait_remainder = 0;
	InputState input_state;

	void initialize_window();
	void initialize_video();
	void initialize_audio();
	void coroutine_entry_point(yielder_t &);
	bool handle_events();
public:
	Pimpl(Engine &);
	~Pimpl();
	void run();
	XorShift128 &get_prng(){
		return this->prng;
	}
	Renderer &get_renderer(){
		return *this->renderer;
	}
	void yield();
	void wait(double s);
	double get_clock();
	void require_redraw();
	DEFINE_GETTER(input_state)

	static const int screen_scale = 4;
};
