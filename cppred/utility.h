#pragma once
#include "common_types.h"
#include "HighResolutionClock.h"
#include <array>
#include <vector>
#include <map>
#include <thread>
#include <boost/coroutine2/all.hpp>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#define BITMAP(x) (bits_from_u32<0x##x>::value)

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

template <typename K, typename V>
const V &find_in_constant_map(const std::map<K, V> &map, const K &key){
	auto it = map.find(key);
	if (it == map.end())
		throw std::exception();
	return it->second;
}

struct Point{
	int x, y;

	Point() = default;
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
};

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

class Coroutine{
public:
	typedef std::function<void(Coroutine &)> entry_point_t;
	typedef std::function<void()> on_yield_t;
private:
	HighResolutionClock clock;
	std::thread::id resume_thread_id;
	typedef boost::coroutines2::asymmetric_coroutine<void>::pull_type coroutine_t;
	typedef boost::coroutines2::asymmetric_coroutine<void>::push_type yielder_t;
	std::unique_ptr<coroutine_t> coroutine;
	on_yield_t on_yield;
	entry_point_t entry_point;
	yielder_t *yielder = nullptr;
	bool first_run = true;
	double wait_remainder = 0;
public:
	Coroutine(entry_point_t &&entry_point);
	bool resume();
	void yield();
	void wait(double seconds);
	void set_on_yield(on_yield_t &&){
		this->on_yield = std::move(on_yield);
	}
	void clear_on_yield(){
		this->on_yield = on_yield_t();
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
	std::uint32_t read_varint(){
		return ::read_varint(this->buffer, this->offset, this->size);
	}
	std::int32_t read_signed_varint(){
		return ::read_signed_varint(this->buffer, this->offset, this->size);
	}
	std::vector<byte_t> read_buffer(){
		return ::read_buffer(this->buffer, this->offset, this->size);
	}
	bool empty() const{
		return this->offset >= this->size;
	}
};
