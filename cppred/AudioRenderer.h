#pragma once
#include "SoundGenerators.h"
#include "PublishingResource.h"
#include "AudioData.h"
#include <fstream>
#include <SDL_hints.h>

//#define OUTPUT_AUDIO_TO_FILE

struct Panning{
	bool left = true,
		right = true,
		either = true;
};

class AudioDevice;

class AudioRenderer{
	std::mutex mutex;
	std::uint64_t expected_frame = 0;
protected:
	AudioDevice *device;
	virtual AudioFrame *get_current_frame() = 0;
	virtual void return_used_frame(AudioFrame *frame) = 0;
public:
	AudioRenderer(AudioDevice &device);
	virtual ~AudioRenderer();
	virtual void update(double now) = 0;
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
	virtual byte_t get_NR11() const = 0;
	virtual byte_t get_NR12() const = 0;
	virtual byte_t get_NR13() const = 0;
	virtual byte_t get_NR14() const = 0;
	virtual byte_t get_NR21() const = 0;
	virtual byte_t get_NR22() const = 0;
	virtual byte_t get_NR23() const = 0;
	virtual byte_t get_NR24() const = 0;
	virtual byte_t get_NR31() const = 0;
	virtual byte_t get_NR32() const = 0;
	virtual byte_t get_NR33() const = 0;
	virtual byte_t get_NR34() const = 0;
	virtual byte_t get_NR41() const = 0;
	virtual byte_t get_NR42() const = 0;
	virtual byte_t get_NR43() const = 0;
	virtual byte_t get_NR44() const = 0;
	virtual byte_t get_NR50() const = 0;
	virtual byte_t get_NR51() const = 0;
	virtual byte_t get_NR52() const = 0;
	virtual void copy_voluntary_wave(const void *buffer) = 0;

	void write_data_to_device(Uint8 *stream, int len, StereoSampleFinal *spillover_buffer, size_t &spillover_buffer_size);
};
