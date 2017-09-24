#pragma once
#include <memory>
#include "common_types.h"
#include "utility.h"

class XorShift128;
class Renderer;

class InputState{
	byte_t value;
	InputState(byte_t value){
		this->value = value;
	}
public:
	InputState(){
		this->value = 0;
	}
	InputState(const InputState &other){
		*this = other;
	}
	const InputState &operator=(const InputState &other){
		this->value = other.value;
		return *this;
	}
	bool operator==(const InputState &other) const{
		return this->value == other.value;
	}
	bool operator!=(const InputState &other) const{
		return !(*this == other);
	}
	InputState operator&(const InputState &is) const{
		return this->value & is.value;
	}
	InputState operator~() const{
		return ~this->value;
	}
	DEFINE_GETTER_SETTER(value);

	static const byte_t mask_a      = 1 << 0;
	static const byte_t mask_b      = 1 << 1;
	static const byte_t mask_start  = 1 << 2;
	static const byte_t mask_select = 1 << 3;
	static const byte_t mask_up     = 1 << 4;
	static const byte_t mask_down   = 1 << 5;
	static const byte_t mask_left   = 1 << 6;
	static const byte_t mask_right  = 1 << 7;

#define DEFINE_InputState_GETTER_SETTER(x) \
	bool get_##x() const{ \
		return !!(this->value & mask_##x); \
	} \
	void set_##x(bool value){ \
		if (value) \
			this->value |= mask_##x; \
		else \
			this->value &= 0xFF ^ mask_##x; \
	}

	DEFINE_InputState_GETTER_SETTER(a)
	DEFINE_InputState_GETTER_SETTER(b)
	DEFINE_InputState_GETTER_SETTER(start)
	DEFINE_InputState_GETTER_SETTER(select)
	DEFINE_InputState_GETTER_SETTER(up)
	DEFINE_InputState_GETTER_SETTER(down)
	DEFINE_InputState_GETTER_SETTER(left)
	DEFINE_InputState_GETTER_SETTER(right)
};

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
	void wait_exactly_one_frame(){
		this->yield();
	}
	double get_clock();
	void require_redraw();
	const InputState &get_input_state();

	static const int dmg_clock_frequency = 1 << 22;
	static const int dmg_display_period = 70224;
	static const double logical_refresh_rate;
	static const double logical_refresh_period;
};
