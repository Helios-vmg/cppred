#pragma once
#include "PublishingResource.h"
#include "threads.h"
#include <SDL.h>
#include <cstdint>
#include <thread>
#include <mutex>
#include <atomic>
#include "common_types.h"

class Engine;

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
	StereoSampleFinal buffer[length];
};

class AbstractAudioRenderer{
public:
	virtual ~AbstractAudioRenderer(){}
	virtual void set_NR10(byte_t) = 0;
	virtual void set_NR11(byte_t) = 0;
	virtual void set_NR12(byte_t) = 0;
	virtual void set_NR13(byte_t) = 0;
	virtual void set_NR14(byte_t) = 0;
	virtual void set_NR21(byte_t) = 0;
	virtual void set_NR22(byte_t) = 0;
	virtual void set_NR23(byte_t) = 0;
	virtual void set_NR24(byte_t) = 0;
	virtual void set_NR30(byte_t) = 0;
	virtual void set_NR31(byte_t) = 0;
	virtual void set_NR32(byte_t) = 0;
	virtual void set_NR33(byte_t) = 0;
	virtual void set_NR34(byte_t) = 0;
	virtual void set_NR41(byte_t) = 0;
	virtual void set_NR42(byte_t) = 0;
	virtual void set_NR43(byte_t) = 0;
	virtual void set_NR44(byte_t) = 0;
	virtual void set_NR50(byte_t) = 0;
	virtual void set_NR51(byte_t) = 0;
	virtual void set_NR52(byte_t) = 0;
	virtual byte_t get_NR11() = 0;
	virtual byte_t get_NR12() = 0;
	virtual byte_t get_NR13() = 0;
	virtual byte_t get_NR14() = 0;
	virtual byte_t get_NR21() = 0;
	virtual byte_t get_NR22() = 0;
	virtual byte_t get_NR23() = 0;
	virtual byte_t get_NR24() = 0;
	virtual byte_t get_NR31() = 0;
	virtual byte_t get_NR32() = 0;
	virtual byte_t get_NR33() = 0;
	virtual byte_t get_NR34() = 0;
	virtual byte_t get_NR41() = 0;
	virtual byte_t get_NR42() = 0;
	virtual byte_t get_NR43() = 0;
	virtual byte_t get_NR44() = 0;
	virtual byte_t get_NR50() = 0;
	virtual byte_t get_NR51() = 0;
	//Assumption: size of buffer is 16.
	virtual void copy_voluntary_wave(const void *buffer) = 0;
};

class AudioProgram{
public:
	virtual void update(double now) = 0;
};

class AudioRenderer : public AbstractAudioRenderer{
	Engine *engine;
	SDL_AudioDeviceID audio_device = 0;
	std::mutex mutex;
	std::uint64_t next_frame = 0;
	std::atomic<bool> continue_running;
	std::unique_ptr<std::thread> thread;
	SDL_TimerID timer_id = 0;
	Event timer_event;
	class ActualRenderer;
	std::unique_ptr<ActualRenderer> renderer;

	static Uint32 SDLCALL timer_callback(Uint32 interval, void *param);
	static void SDLCALL audio_callback(void *userdata, Uint8 *stream, int len);
	AudioFrame *get_current_frame();
	void return_used_frame(AudioFrame *);
	void processor(AudioProgram &program);
public:
	AudioRenderer(Engine &);
	~AudioRenderer();
	void start_audio_processing(AudioProgram &);
	void set_NR10(byte_t) override;
	void set_NR11(byte_t) override;
	void set_NR12(byte_t) override;
	void set_NR13(byte_t) override;
	void set_NR14(byte_t) override;
	void set_NR21(byte_t) override;
	void set_NR22(byte_t) override;
	void set_NR23(byte_t) override;
	void set_NR24(byte_t) override;
	void set_NR30(byte_t) override;
	void set_NR31(byte_t) override;
	void set_NR32(byte_t) override;
	void set_NR33(byte_t) override;
	void set_NR34(byte_t) override;
	void set_NR41(byte_t) override;
	void set_NR42(byte_t) override;
	void set_NR43(byte_t) override;
	void set_NR44(byte_t) override;
	void set_NR50(byte_t) override;
	void set_NR51(byte_t) override;
	void set_NR52(byte_t) override;
	byte_t get_NR11() override;
	byte_t get_NR12() override;
	byte_t get_NR13() override;
	byte_t get_NR14() override;
	byte_t get_NR21() override;
	byte_t get_NR22() override;
	byte_t get_NR23() override;
	byte_t get_NR24() override;
	byte_t get_NR31() override;
	byte_t get_NR32() override;
	byte_t get_NR33() override;
	byte_t get_NR34() override;
	byte_t get_NR41() override;
	byte_t get_NR42() override;
	byte_t get_NR43() override;
	byte_t get_NR44() override;
	byte_t get_NR50() override;
	byte_t get_NR51() override;
	void copy_voluntary_wave(const void *buffer) override;
};
