#pragma once
#include "common_types.h"
#include <array>
#include <vector>

#define BITMAP(x) (bits_from_u32<0x##x>::value)

#define DEFINE_GETTER(x) \
	const decltype(x) &get_##x() const{ \
		return this->x; \
	}

#define DEFINE_NON_CONST_GETTER(x) \
	decltype(x) &get_##x(){ \
		return this->x; \
	}

#define DEFINE_GETTER_SETTER(x) \
	DEFINE_GETTER(x) \
	void set_##x(const decltype(x) &value){ \
		this->x = value; \
	}

typedef std::array<std::uint32_t, 4> xorshift128_state;

template <typename T>
using iterator_pair = std::pair<typename T::iterator, typename T::iterator>;

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

template <typename T>
void fill(std::vector<T> &vector, const T &value){
	std::fill(vector.begin(), vector.end(), value);
}

//Returns true if all the bits that are set in the mask are also set in the value.
template <typename T1, typename T2>
bool check_flag(const T1 &value, const T2 &mask){
	return (value & mask) == mask;
}

xorshift128_state get_seed();
int euclidean_modulo_u(int n, int mod);
int euclidean_modulo(int n, int mod);
int cast_round(double);
