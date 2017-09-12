#include "utility.h"

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
