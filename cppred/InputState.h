#pragma once

#include "utility.h"

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
	bool any_direction() const{
		return !!(this->value & any_direction_mask);
	}
	bool any_main_button() const{
		return !!(this->value & (mask_a | mask_b));
	}
	bool any_secondary_button() const{
		return !!(this->value & (mask_start | mask_select));
	}
	DEFINE_GETTER_SETTER(value);

	static const int offset_a = 0;
	static const int offset_b = 1;
	static const int offset_start = 2;
	static const int offset_select = 3;
	static const int offset_up = 4;
	static const int offset_down = 5;
	static const int offset_left = 6;
	static const int offset_right = 7;
	static const byte_t mask_a = 1 << offset_a;
	static const byte_t mask_b = 1 << offset_b;
	static const byte_t mask_start = 1 << offset_start;
	static const byte_t mask_select = 1 << offset_select;
	static const byte_t mask_up = 1 << offset_up;
	static const byte_t mask_down = 1 << offset_down;
	static const byte_t mask_left = 1 << offset_left;
	static const byte_t mask_right = 1 << offset_right;
	static const byte_t any_direction_mask = mask_up | mask_down | mask_left | mask_right;

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
