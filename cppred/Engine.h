#pragma once
#include <memory>

class XorShift128;
class Renderer;

class Engine{
	class Pimpl;
	std::unique_ptr<void, void (*)(void *)> pimpl;
	Pimpl &get_pimpl();
public:
	Engine();
	~Engine();
	Engine(const Engine &) = delete;
	Engine(Engine &&other): pimpl(std::move(other.pimpl)){}
	void operator=(const Engine &) = delete;
	void operator=(Engine &&) = delete;
	void run();
	XorShift128 &get_prng();
	Renderer &get_renderer();
	void yield();
	void wait(double seconds);
	void wait_frames(int frames);
	double get_clock();

	static const int dmg_clock_frequency = 1 << 22;
	static const int dmg_display_period = 70224;
	static const double logical_refresh_rate;
	static const double logical_refresh_period;
};
