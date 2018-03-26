#pragma once
#include <functional>
#include <memory>

class AbstractClock;
class PausableClock;

class Coroutine{
public:
	typedef std::function<void(Coroutine &)> entry_point_t;
	typedef std::function<void()> on_yield_t;
private:
	class Pimpl;
	std::unique_ptr<Pimpl> pimpl;

public:
	Coroutine(const std::string &name, AbstractClock &base_clock, entry_point_t &&entry_point);
	Coroutine(const std::string &name, entry_point_t &&entry_point);
	~Coroutine();
	bool resume();
	void yield();
	void wait(double seconds);
	void wait_frames(int frames);
	void set_on_yield(on_yield_t &&on_yield);
	void clear_on_yield();
	static Coroutine *get_current_coroutine_ptr();
	static Coroutine &get_current_coroutine();
	PausableClock &get_clock();
};
