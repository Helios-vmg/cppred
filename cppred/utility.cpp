#include "utility.h"
#include "CppRedStructs.h"
#include "../CodeGeneration/output/bitmaps.h"

std::uint32_t xorshift128(xorshift128_state &state){
	auto x = state[3];
	x ^= x << 11;
	x ^= x >> 8;
	state[3] = state[2];
	state[2] = state[1];
	state[1] = state[0];
	x ^= state[0];
	x ^= state[0] >> 19;
	state[0] = x;
	return x;
}

byte_t calculate_checksum(const void *void_data, size_t size){
	auto data = (const byte_t *)void_data;
	size_t ret = 0;
	for (size_t i = 0; i < size; i++)
		ret += data[i];
	return (byte_t)(~ret & 0xFF);
}

unsigned count_set_bits(const byte_t *src, size_t size){
	unsigned ret = 0;
	for (size_t i = size; i--; src++){
		auto byte = *src;
		for (int j = 8; j--;){
			ret += !!(byte & 1);
			byte >>= 1;
		}
	}
	return ret;
}

InputBitmap_struct operator&(const InputBitmap_struct &a, const InputBitmap_struct &b){
	InputBitmap_struct ret;
	ret.button_a = a.button_a && b.button_a;
	ret.button_b = a.button_b && b.button_b;
	ret.button_select = a.button_select && b.button_select;
	ret.button_start = a.button_start && b.button_start;
	ret.button_right = a.button_right && b.button_right;
	ret.button_left = a.button_left && b.button_left;
	ret.button_up = a.button_up && b.button_up;
	ret.button_down = a.button_down && b.button_down;
	return ret;
}

bool any_button_pressed(const InputBitmap_struct &bitmap){
	return !!(bitmap.button_a | bitmap.button_b | bitmap.button_select | bitmap.button_start | bitmap.button_left | bitmap.button_right | bitmap.button_up | bitmap.button_down);
}
