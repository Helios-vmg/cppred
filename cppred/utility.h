#pragma once
#include "common_types.h"
#include <array>

#define BITMAP(x) (bits_from_u32<0x##x>::value)

typedef std::array<std::uint32_t, 4> xorshift128_state;

class XorShift128{
	xorshift128_state state;
public:
	XorShift128(const xorshift128_state &seed): state(seed){}
	XorShift128(xorshift128_state &&seed): state(std::move(seed)){}
	std::uint32_t operator()();
	void generate_block(void *buffer, size_t size);
};

template <typename T, size_t N>
constexpr size_t array_length(T (&)[N]){
	return N;
}

template <std::uint32_t N, int M>
struct bits_from_u32_helper{
	static const byte_t value = bits_from_u32_helper<N, M + 1>::value |
		(!!((N >> (4 * M)) & 0xF) << M);
};

template <std::uint32_t N>
struct bits_from_u32_helper<N, 8>{
	static const byte_t value = 0;
};

template <std::uint32_t N>
struct bits_from_u32{
	static const byte_t value = bits_from_u32_helper<N, 0>::value;
};

template <typename T>
void deleter(void *p){
	delete (T *)p;
}

template <typename T, size_t N>
void fill(T (&array)[N], const T &value){
	std::fill(array, array + N, value);
}

xorshift128_state get_seed();
