#include "EnginePrivate.h"

const double Engine::logical_refresh_rate = (double)dmg_clock_frequency / dmg_display_period;
const double Engine::logical_refresh_period = (double)dmg_display_period / dmg_clock_frequency;

Engine::Engine(): pimpl(nullptr, deleter<Pimpl>){
	this->pimpl.reset(new Pimpl(*this));
}

Engine::~Engine() = default;

Engine::Pimpl &Engine::get_pimpl(){
	return *(Pimpl *)this->pimpl.get();
}

void Engine::run(){
	this->get_pimpl().run();
}

XorShift128 &Engine::get_prng(){
	return this->get_pimpl().get_prng();
}

Renderer &Engine::get_renderer(){
	return this->get_pimpl().get_renderer();
}

void Engine::yield(){
	this->get_pimpl().yield();
}

void Engine::wait(double seconds){
	this->get_pimpl().wait(seconds);
}

double Engine::get_clock(){
	return this->get_pimpl().get_clock();
}

void Engine::wait_frames(int frames){
	this->wait(frames * logical_refresh_period);
}

const InputState &Engine::get_input_state(){
	return this->get_pimpl().get_input_state();
}
