#pragma once

#include "CommonTypes.h"
#include "PublishingResource.h"
#include <fstream>

class Gameboy;
class SoundController;

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

class ClockDivider{
public:
#ifdef USE_STD_FUNCTION
	typedef std::function<void(std::uint64_t)> callback_t;
#else
	typedef void(*callback_t)(void *, std::uint64_t);
#endif
private:
	callback_t callback;
#ifndef USE_STD_FUNCTION
	void *user_data;
#endif
	unsigned src_frequency_power;
	std::uint64_t dst_frequency;
	std::uint64_t last_update;
public:
	ClockDivider();
#ifdef USE_STD_FUNCTION
	ClockDivider(unsigned src_frequency_power, std::uint64_t dst_frequency, callback_t &&callback);
	void configure(unsigned src_frequency_power, std::uint64_t dst_frequency, callback_t &&callback);
#else
	ClockDivider(unsigned src_frequency_power, std::uint64_t dst_frequency, callback_t callback, void *user_data);
	void configure(unsigned src_frequency_power, std::uint64_t dst_frequency, callback_t callback, void *user_data);
#endif
	void update(std::uint64_t);
	void reset();
};

class WaveformGenerator{
protected:
	SoundController *parent;

	byte_t registers[5];

	//Sound length
	unsigned sound_length = 0,
		shadow_sound_length = 0;
	bool length_enable = false;

	virtual void trigger_event();
	virtual bool enabled() const;
public:
	WaveformGenerator(SoundController &parent);
	virtual ~WaveformGenerator(){}
	virtual void update_state_before_render(std::uint64_t time){}
	virtual intermediate_audio_type render(std::uint64_t time) const = 0;
	virtual void set_register1(byte_t);
	virtual void set_register2(byte_t){}
	virtual void set_register3(byte_t){}
	virtual void set_register4(byte_t);
	virtual byte_t get_register1() const{
		return 0xFF;
	}
	virtual byte_t get_register2() const{
		return 0xFF;
	}
	virtual byte_t get_register3() const{
		return 0xFF;
	}
	byte_t get_register4() const;
	void length_counter_event();
	bool length_counter_has_not_finished() const;
};

class EnvelopedGenerator : public WaveformGenerator{
protected:
	//Envelope
	int envelope_sign = 0;
	unsigned envelope_period = 0;
	unsigned envelope_time = 0;
	int volume = 0;

	virtual bool enabled() const override;
	virtual void trigger_event() override;
	intermediate_audio_type render_from_bit(bool signal) const;
	void load_volume_from_register();
public:
	EnvelopedGenerator(SoundController &parent):
		WaveformGenerator(parent){}
	virtual ~EnvelopedGenerator(){}
	void volume_event();
	virtual void set_register2(byte_t value) override;
	virtual byte_t get_register2() const override;
};

class FrequenciedGenerator{
protected:
	unsigned frequency = 0;
	unsigned period = 0;
	std::uint64_t reference_time = 0;
	unsigned cycle_position = 0;
	unsigned reference_cycle_position = 0;
	const decltype(reference_time) undefined_reference_time = std::numeric_limits<decltype(reference_time)>::max();
	const decltype(reference_cycle_position) undefined_reference_cycle_position = std::numeric_limits<decltype(reference_cycle_position)>::max();

	template <unsigned Shift>
	void advance_cycle(std::uint64_t time);
	void frequency_change(unsigned old_frequency);
	virtual unsigned get_period() = 0;
	void write_register3_frequency(byte_t value);
	void write_register4_frequency(byte_t value);
	void reset_references();
public:
	virtual ~FrequenciedGenerator(){}
};

class Square2Generator : public EnvelopedGenerator, public FrequenciedGenerator{
protected:
	unsigned selected_duty = 2;
	static const byte_t duties[4];

	unsigned get_period() override;
	virtual bool enabled() const override;
	void trigger_event() override;
public:
	Square2Generator(SoundController &parent) :
		EnvelopedGenerator(parent){}
	virtual ~Square2Generator(){}
	void update_state_before_render(std::uint64_t time) override;
	intermediate_audio_type render(std::uint64_t time) const override;

	virtual void set_register1(byte_t value) override;
	virtual void set_register3(byte_t value) override;
	virtual void set_register4(byte_t value) override;
	virtual byte_t get_register1() const override;
	virtual byte_t get_register3() const override;
};

class Square1Generator : public Square2Generator{
	unsigned sweep_period = 0;
	unsigned sweep_time = 0;
	int sweep_sign = 0;
	unsigned sweep_shift = 0;
	int shadow_frequency = 0;
	static const unsigned audio_disabled_by_sweep = 2048;
	std::uint64_t last_sweep = 0;

	bool enabled() const override;
	void trigger_event() override;
public:
	Square1Generator(SoundController &parent) :
		Square2Generator(parent){}
	void set_register0(byte_t value);
	byte_t get_register0() const;
	void sweep_event(bool force = false);
};

class NoiseGenerator : public EnvelopedGenerator{
	unsigned width_mode = 0;
	unsigned noise_register = 1;
	bool output = true;

	ClockDivider noise_scheduler;

	static void noise_update_event(void *, std::uint64_t);
	void noise_update_event();
	void trigger_event() override;
public:
	NoiseGenerator(SoundController &parent) :
		EnvelopedGenerator(parent){}
	void set_register3(byte_t value) override;
	intermediate_audio_type render(std::uint64_t time) const override;
	void update_state_before_render(std::uint64_t time) override;
};

class VoluntaryWaveGenerator : public WaveformGenerator, public FrequenciedGenerator{
	bool dac_power = false;
	unsigned volume_shift = 0;
	byte_t wave_buffer[32];
	byte_t sample_register = 0;

	bool enabled() const override;
	void trigger_event() override;
public:
	VoluntaryWaveGenerator(SoundController &parent);
	void update_state_before_render(std::uint64_t time) override;
	intermediate_audio_type render(std::uint64_t time) const override;
	unsigned get_period() override;

	void set_register0(byte_t);
	void set_register1(byte_t) override;
	void set_register2(byte_t) override;
	void set_register3(byte_t) override;
	void set_register4(byte_t) override;
	void set_wave_table(unsigned position, byte_t value);

	byte_t get_register0() const;
	byte_t get_register1() const override;
	byte_t get_register2() const override;
	byte_t get_register3() const override;
};

class CapacitorFilter{
	intermediate_audio_type state = 0;
public:
	intermediate_audio_type update(intermediate_audio_type in);
};

class SoundController{
	Gameboy *system;
	unsigned current_frame_position = 0;
	QueuedPublishingResource<AudioFrame> publishing_frames;
	std::uint64_t frame_no = 0;
	std::uint64_t audio_turned_on_at = 0;
	std::uint64_t current_clock;
	ClockDivider frame_sequencer_clock,
		audio_sample_clock;
	CapacitorFilter filter_left,
		filter_right;

	//Control registers.
	byte_t NR50 = 0;
	byte_t NR51 = 0;
	bool master_toggle = false;

	struct Panning{
		bool left = true,
			right = true,
			either = true;
	};

	Panning stereo_panning[4];

	unsigned left_volume = 0,
		right_volume = 0;

	std::unique_ptr<std::ofstream> output_file;
	std::unique_ptr<std::ofstream> output_files_by_channel[4];
	std::unique_ptr<AudioFrame> output_buffers_by_channel[4];
	std::uint64_t speed_multiplier = 0x10000;
	std::uint64_t speed_counter_a = 0;
	std::uint64_t speed_counter_b = 0;
	std::uint64_t internal_sample_counter = 0;
	StereoSampleFinal last_sample;

	StereoSampleIntermediate render_square1(std::uint64_t time);
	StereoSampleIntermediate render_square2(std::uint64_t time);
	StereoSampleIntermediate render_voluntary(std::uint64_t time);
	StereoSampleIntermediate render_noise(std::uint64_t time);
	void initialize_new_frame();
	static void sample_callback(void *, std::uint64_t);
	static void frame_sequencer_callback(void *, std::uint64_t);
	StereoSampleFinal compute_sample();
	void write_sample(StereoSampleFinal *&buffer);
	void sample_callback(std::uint64_t);
	void frame_sequencer_callback(std::uint64_t);
	void length_counter_event();
	void volume_event();
	void sweep_event();
public:
	Square1Generator square1;
	Square2Generator square2;
	VoluntaryWaveGenerator wave;
	NoiseGenerator noise;

	SoundController(Gameboy &);
	void update(double speed_multiplier, bool speed_changed);
	AudioFrame *get_current_frame();
	void return_used_frame(AudioFrame *);
	std::uint64_t get_current_clock() const{
		return this->current_clock;
	}

	void set_NR50(byte_t);
	void set_NR51(byte_t);
	void set_NR52(byte_t);
	byte_t get_NR50() const{
		return this->NR50;
	}
	byte_t get_NR51() const{
		return this->NR51;
	}
	byte_t get_NR52() const;
};
