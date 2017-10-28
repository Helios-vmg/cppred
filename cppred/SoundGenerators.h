#pragma once
#include "common_types.h"
#include "AudioData.h"
#include <limits>

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
	byte_t registers[5];

	//Sound length
	unsigned sound_length = 0,
		shadow_sound_length = 0;
	bool length_enable = false;

	virtual void trigger_event();
	virtual bool enabled() const;
public:
	WaveformGenerator();
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
	void set_register0(byte_t value);
	byte_t get_register0() const;
	void sweep_event(bool force = false);
};

class NoiseGenerator : public EnvelopedGenerator{
	unsigned width_mode = 14;
	unsigned noise_register = 1;
	bool output = true;

	ClockDivider noise_scheduler;

	static void noise_update_event(void *, std::uint64_t);
	void noise_update_event();
	void trigger_event() override;
public:
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
	VoluntaryWaveGenerator();
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
