#pragma once
#include <cstdint>

static const unsigned gb_cpu_frequency_power = 22;
static const unsigned gb_cpu_frequency = 1 << gb_cpu_frequency_power;
static const unsigned sampling_frequency = 44100;

template <typename T>
struct basic_StereoSample{
	typedef T type;
	T left, right;

	const basic_StereoSample<T> &operator+=(const basic_StereoSample<T> &other){
		this->left += other.left;
		this->right += other.right;
		return *this;
	}
	const basic_StereoSample<T> operator-=(const basic_StereoSample<T> &other){
		this->left -= other.left;
		this->right -= other.right;
		return *this;
	}
	basic_StereoSample<T> operator+(const basic_StereoSample<T> &other) const{
		auto ret = *this;
		ret += other;
		return ret;
	}
	basic_StereoSample<T> operator-(const basic_StereoSample<T> &other) const{
		auto ret = *this;
		ret -= other;
		return ret;
	}
	basic_StereoSample<T> operator-() const{
		auto ret = *this;
		ret.left = -ret.left;
		ret.right = -ret.right;
		return ret;
	}
	const basic_StereoSample<T> &operator/=(T n){
		this->left /= n;
		this->right /= n;
		return *this;
	}
	basic_StereoSample<T> operator/(T n) const{
		auto ret = *this;
		ret /= n;
		return ret;
	}
	const basic_StereoSample<T> &operator*=(T n){
		this->left *= n;
		this->right *= n;
		return *this;
	}
	basic_StereoSample<T> operator*(T n) const{
		auto ret = *this;
		ret *= n;
		return ret;
	}
};

//#define USE_FLOAT_AUDIO
//#define USE_STD_FUNCTION

#ifdef USE_FLOAT_AUDIO
typedef float intermediate_audio_type;
#else
typedef std::int32_t intermediate_audio_type;
#endif
typedef basic_StereoSample<std::int16_t> StereoSampleFinal;
typedef basic_StereoSample<intermediate_audio_type> StereoSampleIntermediate;

basic_StereoSample<std::int16_t> convert(const basic_StereoSample<intermediate_audio_type> &);

struct AudioFrame{
	static const unsigned length = 1024;
	std::uint64_t frame_no;
	bool active;
	StereoSampleFinal buffer[length];
};
