#pragma once
#include "CommonTypes.h"
#include <mutex>
#include <memory>
#include <atomic>
#include <cstring>

class Gameboy;

struct InputState{
	byte_t up, down, left, right, a, b, start, select;
	InputState(){
		memset(this, 0x00, sizeof(*this));
	}
	bool operator==(const InputState &other) const{
		return
			(this->up == other.up) &
			(this->down == other.down) &
			(this->left == other.left) &
			(this->right == other.right) &
			(this->a == other.a) &
			(this->b == other.b) &
			(this->start == other.start) &
			(this->select == other.select);
	}
	bool operator!=(const InputState &other) const{
		return !(*this == other);
	}
};

class UserInputController{
	Gameboy *system;
	std::atomic<InputState *> input_state;
	byte_t saved_state = 0;
	bool state_changed = false;
	std::atomic<bool> button_down;

	static const byte_t pin10_mask = 1 << 0;
	static const byte_t pin11_mask = 1 << 1;
	static const byte_t pin12_mask = 1 << 2;
	static const byte_t pin13_mask = 1 << 3;
	static const byte_t pin14_mask = 1 << 4;
	static const byte_t pin15_mask = 1 << 5;
public:
	UserInputController(Gameboy &system);
	~UserInputController();
	void set_input_state(InputState *state, bool button_down, bool button_up);
	void request_input_state(byte_t select);
	byte_t get_requested_input_state(){
		return this->saved_state;
	}
	bool get_button_down();
};
