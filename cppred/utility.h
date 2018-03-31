#pragma once
#include "common_types.h"
#include "HighResolutionClock.h"
#ifndef HAVE_PCH
#include <vector>
#include <map>
#include <string>
#include <limits>
#include <iostream>
#include <cstring>
#endif

#define BITMAP(x) (bits_from_u32<0x##x>::value)

//#define CONCAT_HELPER(x) x
//#define CONCAT(x, y) CONCAT_HELPER(x)##CONCAT_HELPER(y)
//#define LOCK_MUTEX(x) std::lock_guard<decltype(x)> CONCAT(lg_,__COUNTER__)(x)
#define LOCK_MUTEX(x) std::lock_guard<decltype(x)> lg_##__COUNTER__(x)

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

struct xorshift128_state{
	std::uint32_t data[4];
};

class XorShift128{
	xorshift128_state state;
	std::uint32_t gen();
	std::uint32_t gen(std::uint32_t max);
public:
	XorShift128(const xorshift128_state &seed): state(seed){}
	XorShift128(xorshift128_state &&seed): state(std::move(seed)){}
	//Generates in range [0; end)
	std::uint32_t operator()(std::uint32_t end = 0);
	//Generates in range [begin; end)
	std::uint32_t operator()(std::uint32_t begin, std::uint32_t end){
		return (*this)(end - begin) + begin;
	}
	void generate_block(void *buffer, size_t size);
	double generate_double();
	template <typename T>
	void generate(T &dst){
		dst = (T)(*this)(std::numeric_limits<T>::max() + 1);
	}
	void generate(std::uint64_t &dst){
		dst = (*this)();
		dst <<= 32;
		dst |= (*this)();
	}
};

template <typename T, size_t N>
constexpr size_t array_length(T (&)[N]){
	return N;
}

template <typename T, size_t N>
constexpr size_t array_length(const std::array<T, N> &){
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

template <typename T, typename T2, size_t N>
void fill(T (&array)[N], const T2 &value){
	std::fill(array, array + N, (T)value);
}

template <typename T, typename T2, size_t N>
void fill(std::array<T, N> &array, const T2 &value){
	std::fill(array.data(), array.data() + N, (T)value);
}

template <typename T, typename T2>
void fill(std::vector<T> &vector, const T2 &value){
	std::fill(vector.begin(), vector.end(), (T)value);
}

//Returns true if all the bits that are set in the mask are also set in the value.
template <typename T1, typename T2>
bool check_flag(const T1 &value, const T2 &mask){
	return (value & mask) == mask;
}

inline char make_apostrophe(char c){
	return c + 128;
}

constexpr std::uint32_t bit(std::uint32_t i){
	return (std::uint32_t)1 << i;
}

template <typename T1, typename T2>
void set_and_swap(T1 &dst, T1 &src, const T2 &null_value){
	dst = src;
	src = null_value;
}

template<class It, class F>
It find_first_true(It begin, It end, const F &f){
	if (begin >= end)
		return end;
	if (f(*begin))
		return begin;
	auto diff = end - begin;
	while (diff > 1){
		auto pivot = begin + diff / 2;
		if (!f(*pivot))
			begin = pivot;
		else
			end = pivot;
		diff = end - begin;
	}
	return end;
}

template<class It, class F>
It find_first_true_unsorted(It begin, It end, const F &f){
	if (begin >= end)
		return end;
	for (; begin != end; ++begin)
		if (f(*begin))
			return begin;
	return end;
}

template<class T, size_t N, class F>
T *find_first_true_unsorted(T (&array)[N], const F &f){
	auto it = find_first_true_unsorted(array, array + N, f);
	if (it == array + N)
		return nullptr;
	return it;
}

xorshift128_state get_seed();
int euclidean_modulo_u(int n, int mod);
int euclidean_modulo(int n, int mod);
int cast_round(double);
std::uint64_t cast_round_u64(double);
std::uint32_t read_u32(const void *);
std::uint32_t read_varint(const byte_t *buffer, size_t &offset, size_t size);
std::int32_t read_signed_varint(const byte_t *buffer, size_t &offset, size_t size);
std::string read_string(const byte_t *buffer, size_t &offset, size_t size);
std::vector<byte_t> read_buffer(const byte_t *buffer, size_t &offset, size_t size);
template <typename T>
std::array<char, sizeof(T) * CHAR_BIT> number_to_decimal_string(T value, int right_padding = 0){
	//Note: sizeof(T) * CHAR_BIT is about three times larger than the optimal size.
	//In reality we only need approximately sizeof(T) * CHAR_BIT * log10(2).
	const size_t n = sizeof(T) * CHAR_BIT;
	if (right_padding >= n)
		throw std::runtime_error("number_to_decimal_string(): incorrect usage.");
	std::array<char, n> ret;
	fill(ret, 0);
	auto i = sizeof(T) * CHAR_BIT;
	if (!value)
		ret[--i] = '0';
	else{
		bool negative = value < 0;
		if (negative)
			value = -value;
		while (value){
			ret[--i] = '0' + value % 10;
			value /= 10;
		}
		if (negative)
			ret[--i] = '-';
	}
	while (n - i < right_padding)
		ret[--i] = ' ';
	for (int j = 0; j < n - i; j++)
		ret[j] = ret[i + j];
	ret[n - i] = 0;
	return ret;
}

template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, int>::type
euclidean_modulo(int n, T mod){
	return euclidean_modulo_u(n, (int)mod);
}

template <typename K, typename V>
const V &find_in_constant_map(const std::map<K, V> &map, const K &key){
	auto it = map.find(key);
	if (it == map.end())
		throw std::exception();
	return it->second;
}

struct Point{
	int x, y;

	Point(): x(0), y(0){}
	Point(int x, int y): x(x), y(y){}
	Point operator+(const Point &p) const{
		return { this->x + p.x, this->y + p.y };
	}
	Point operator-(const Point &p) const{
		return { this->x - p.x, this->y - p.y };
	}
	Point operator-() const{
		return { -this->x, -this->y };
	}
	Point operator*(double x) const{
		return { cast_round(this->x * x), cast_round(this->y * x) };
	}
	const Point &operator+=(const Point &other){
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
	const Point &operator-=(const Point &other){
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}
	const Point &operator*=(double x){
		this->x = cast_round(this->x * x);
		this->y = cast_round(this->y * x);
		return *this;
	}
	int multiply_components() const{
		return this->x * this->y;
	}
	bool operator==(const Point &other) const{
		return this->x == other.x && this->y == other.y;
	}
	bool operator!=(const Point &other) const{
		return !(*this == other);
	}
	bool operator<(const Point &other) const{
		if (this->y < other.y)
			return true;
		if (this->y > other.y)
			return false;
		return this->x < other.x;
	}
};

inline std::ostream &operator<<(std::ostream &stream, const Point &p){
	return stream << '(' << p.x << ", " << p.y << ')';
}

enum class Map;

struct WorldCoordinates{
	Map map;
	Point position;

	WorldCoordinates() = default;
	WorldCoordinates(Map map): map(map){}
	WorldCoordinates(Map map, const Point &p): map(map), position(p){}

	WorldCoordinates operator+(const Point &p) const{
		return { this->map, this->position + p };
	}
	WorldCoordinates operator-(const Point &p) const{
		return { this->map, this->position - p };
	}
	WorldCoordinates operator-() const{
		return { this->map, -this->position };
	}
	WorldCoordinates operator*(double x) const{
		return { this->map, this->position * x };
	}
	const WorldCoordinates &operator+=(const Point &other){
		this->position += other;
		return *this;
	}
	const WorldCoordinates &operator-=(const Point &other){
		this->position -= other;
		return *this;
	}
	const WorldCoordinates &operator*=(double x){
		this->position *= x;
		return *this;
	}
};

class BufferReader{
	const byte_t *buffer;
	size_t size;
	size_t offset = 0;
public:
	BufferReader(const byte_t *buffer, size_t size): buffer(buffer), size(size){}
	std::string read_string(){
		return ::read_string(this->buffer, this->offset, this->size);
	}
	std::vector<byte_t> read_string_as_vector();
	std::uint32_t read_varint(){
		return ::read_varint(this->buffer, this->offset, this->size);
	}
	byte_t read_byte();
	std::uint32_t read_u32();
	std::int32_t read_signed_varint(){
		return ::read_signed_varint(this->buffer, this->offset, this->size);
	}
	std::vector<byte_t> read_buffer(){
		return ::read_buffer(this->buffer, this->offset, this->size);
	}
	bool empty() const{
		return this->offset >= this->size;
	}
	size_t remaining_bytes() const{
		return this->size - this->offset;
	}
};

template <typename T>
class iterator_range{
	T b, e;
public:
	iterator_range(const T &begin, const T &end): b(begin), e(end){}
	T begin() const{
		return this->b;
	}
	T end() const{
		return this->e;
	}
};

template <typename T>
auto make_range(const T &xs){
	return iterator_range<decltype(std::begin(xs))>(std::begin(xs), std::end(xs));
}

template <typename T>
auto make_range(T &xs){
	return iterator_range<decltype(std::begin(xs))>(std::begin(xs), std::end(xs));
}

enum class FacingDirection{
	Up = 0,
	Right,
	Down,
	Left,
};

Point direction_to_vector(FacingDirection);
FacingDirection invert_direction(FacingDirection);

template <size_t N>
class FixedBitmap{
public:
	static const size_t size = (N + 7) / 8;
private:
	byte_t data[size];
public:
	FixedBitmap(bool default_value = false){
		memset(this->data, default_value ? 0xFF : 0x00, size);
	}
	FixedBitmap(const FixedBitmap &other) = default;
	const FixedBitmap &operator=(const FixedBitmap &other){
		memcpy(this->data, other.data, size);
		return *this;
	}
	FixedBitmap(FixedBitmap &&other) = delete;
	void operator=(FixedBitmap &&other) = delete;
	void set(size_t index, bool value = true){
		byte_t mask = 1 << (index % 8);
		if (value)
			this->data[index / 8] |= mask;
		else
			this->data[index / 8] &= ~mask;
	}
	bool get(size_t index) const{
		byte_t mask = 1 << (index % 8);
		return !!(this->data[index / 8] & mask);
	}
	void get_data(byte_t (&dst)[size]) const{
		memcpy(dst, this->data, size);
	}
};
