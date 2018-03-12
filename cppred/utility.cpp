#include "utility.h"
#include <random>
#include <cmath>
#include <cstring>
#include <iostream>
#include "Engine.h"

thread_local std::vector<Coroutine *> Coroutine::coroutine_stack;

std::uint32_t XorShift128::gen(){
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

std::uint32_t XorShift128::gen(std::uint32_t max){
	const auto u32_max = std::numeric_limits<std::uint32_t>::max();
	const auto u32_max2 = u32_max / 2;
	const std::uint32_t rand_max = max > u32_max2 ? u32_max2 : u32_max - (u32_max2 + 1) % (max + 1);
    std::uint32_t r;
    do
        r = (*this)();
    while (r > rand_max);
	return max <= u32_max2 ? r % (max + 1) : r;
}

std::uint32_t XorShift128::operator()(std::uint32_t max){
	if (!max)
		return this->gen();
	return this->gen(max - 1);
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

double XorShift128::generate_double(){
	double ret = 0;
	double addend = 0.5;
	auto n = (*this)();
	for (int i = 32; i--;){
		ret += addend;
		addend *= 0.5;
	}
	n = (*this)();
	for (int i = 53 - 32; i--;){
		ret += addend;
		addend *= 0.5;
	}
	return ret;
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
	return (mod - (-n % mod)) % mod;
}

int euclidean_modulo(int n, int mod){
	if (mod < 0)
		mod = -mod;
	return euclidean_modulo_u(n, mod);
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

template <typename T>
typename std::make_signed<T>::type uints_to_ints(T n){
	typedef typename std::make_signed<T>::type s;
	if ((n & 1) == 0)
		return (s)(n / 2);
	return -(s)((n - 1) / 2) - 1;
}

std::int32_t read_signed_varint(const byte_t *buffer, size_t &offset, size_t size){
	return uints_to_ints(read_varint(buffer, offset, size));
}

template <typename DstT, typename ElemT = byte_t>
DstT basic_read_string(const byte_t *buffer, size_t &offset, size_t size){
	DstT ret;
	while (true){
		if (offset >= size)
			throw std::runtime_error("read_string(): Invalid read.");
		auto byte = buffer[offset++];
		if (!byte)
			break;
		ret.push_back((ElemT)byte);
	}
	return ret;
}

std::string read_string(const byte_t *buffer, size_t &offset, size_t size){
	return basic_read_string<std::string, char>(buffer, offset, size);
}

std::vector<byte_t> read_buffer(const byte_t *buffer, size_t &offset, size_t size){
	std::vector<byte_t> ret;
	ret.resize(read_varint(buffer, offset, size));
	if (offset + ret.size() > size)
		throw std::runtime_error("read_buffer(): Invalid read.");
	memcpy(&ret[0], buffer + offset, ret.size());
	offset += ret.size();
	return ret;
}

Coroutine::Coroutine(const std::string &name, entry_point_t &&entry_point):
	Coroutine(name, get_current_coroutine().get_clock(), std::move(entry_point)){}

Coroutine::Coroutine(const std::string &name, AbstractClock &base_clock, entry_point_t &&entry_point):
		name(name),
		clock(base_clock),
		entry_point(std::move(entry_point)){
	this->coroutine_stack.push_back(this);
	this->coroutine.reset(new coroutine_t([this](yielder_t &y){
		this->resume_thread_id = std::this_thread::get_id();
		this->yielder = &y;
		if (this->first_run){
			this->yield();
			this->first_run = false;
		}
		this->entry_point(*this);
	}));
	this->coroutine_stack.pop_back();
}

void Coroutine::yield(){
	if (!this->coroutine_stack.size() || this->coroutine_stack.back() != this)
		throw std::runtime_error("Coroutine::yield() was used incorrectly!");
	if (std::this_thread::get_id() != this->resume_thread_id)
		throw std::runtime_error("Coroutine::yield() must be called from the thread that resumed the coroutine!");
	if (!this->yielder)
		throw std::runtime_error("Coroutine::yield() must be called while the coroutine is active!");
	auto yielder = this->yielder;
	this->yielder = nullptr;
	(*yielder)();
	this->yielder = yielder;
	if (this->on_yield)
		this->on_yield();
}

bool Coroutine::resume(){
	this->resume_thread_id = std::this_thread::get_id();
	if (this->active)
		throw std::runtime_error("Attempting to resume a running coroutine!");
	this->active = true;
	this->coroutine_stack.push_back(this);
	//std::cout << this->name << " RESUMES\n";
	this->clock.resume();
	auto ret = !!(*this->coroutine)();
	//std::cout << this->name << " PAUSES\n";
	this->coroutine_stack.pop_back();
	this->active = false;
	return ret;
}

Coroutine *Coroutine::get_current_coroutine_ptr(){
	if (!Coroutine::coroutine_stack.size())
		return nullptr;
	return Coroutine::coroutine_stack.back();
}

void Coroutine::wait(double s){
	auto target = this->clock.get() + s + this->wait_remainder;
	while (true){
		this->yield();
		this->clock.step();
		auto now = this->clock.get();
		if (now >= target){
			this->wait_remainder = target - now;
			return;
		}
	}
}

void Coroutine::wait_frames(int frames){
	this->wait(frames * Engine::logical_refresh_period);
}

byte_t BufferReader::read_byte(){
	if (this->remaining_bytes() < 1)
		throw std::runtime_error("Buffer too short.");
	return this->buffer[this->offset++];
}

std::uint32_t BufferReader::read_u32(){
	if (this->remaining_bytes() < 4)
		throw std::runtime_error("Buffer too short.");
	auto ret = ::read_u32(this->buffer + this->offset);
	this->offset += 4;
	return ret;
}

std::vector<byte_t> BufferReader::read_string_as_vector(){
	return basic_read_string<std::vector<byte_t>>(this->buffer, this->offset, this->size);
}

Point direction_to_vector(FacingDirection direction){
	static const Point deltas[] = {
		{ 0, -1},
		{ 1,  0},
		{ 0,  1},
		{-1,  0},
	};
	return deltas[(int)direction];
}
