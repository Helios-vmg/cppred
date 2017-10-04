#include "utility.h"
#include <random>
#include <cmath>

std::uint32_t XorShift128::operator()(){
	auto x = this->state[3];
	x ^= x << 11;
	x ^= x >> 8;
	this->state[3] = this->state[2];
	this->state[2] = this->state[1];
	this->state[1] = this->state[0];
	x ^= this->state[0];
	x ^= this->state[0] >> 19;
	this->state[0] = x;
	return x;
}


void XorShift128::generate_block(void *buffer, size_t size){
	auto dst = (std::uint8_t *)buffer;
	for (size_t i = 0; i < size;){
		auto u = (*this)();
		for (int j = 4; j--; i++){
			dst[i] = u & 0xFF;
			u >>= 8;
		}
	}
}

xorshift128_state get_seed(){
	xorshift128_state ret;
	std::random_device rnd;
	for (auto &i : ret)
		i = rnd();
	return ret;
}

int euclidean_modulo_u(int n, int mod){
	if (n >= 0)
		return n % mod;
	return mod - (-n % mod);
}

int euclidean_modulo(int n, int mod){
	if (mod < 0)
		mod = -mod;
	if (n >= 0)
		return n % mod;
	return mod - (-n % mod);
}

int cast_round(double x){
	return (int)round(x);
}

std::uint64_t cast_round_u64(double x){
	return (std::uint64_t)round(x);
}

std::uint32_t read_u32(const void *void_buffer){
	auto buffer = (const byte_t *)void_buffer;
	std::uint32_t ret = 0;
	buffer += 4;
	while (buffer != (const byte_t *)void_buffer){
		ret <<= 8;
		ret |= *--buffer;
	}
	return ret;
}

std::uint32_t read_varint(const byte_t *buffer, size_t &offset, size_t size){
	std::uint32_t ret = 0;
	int shift = 0;
	bool terminate = false;
	do{
		if (offset >= size)
			throw std::runtime_error("read_varint(): Invalid read.");
		auto byte = buffer[offset++];
		terminate = !(byte & BITMAP(10000000));
		byte &= BITMAP(01111111);
		ret |= (std::uint32_t)byte << shift;
		shift += 7;
	}while (!terminate);
	return ret;
}

std::string read_string(const byte_t *buffer, size_t &offset, size_t size){
	std::string ret;
	while (true){
		if (offset >= size)
			throw std::runtime_error("read_string(): Invalid read.");
		auto byte = buffer[offset++];
		if (!byte)
			break;
		ret.push_back((char)byte);
	}
	return ret;
}
