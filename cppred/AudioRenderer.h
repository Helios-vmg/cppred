#pragma once
#include "Audio.h"
#include "SoundGenerators.h"
#include "PublishingResource.h"
#include <fstream>

//#define OUTPUT_AUDIO_TO_FILE

struct Panning{
	bool left = true,
		right = true,
		either = true;
};

class AudioRenderer{
	unsigned current_frame_position = 0;
	std::uint64_t frame_no = 0;
protected:
	QueuedPublishingResource<AudioFrame> publishing_frames;

	void write_sample(StereoSampleFinal *&buffer, const StereoSampleFinal &sample);
	void initialize_new_frame();
public:
	virtual ~AudioRenderer(){}
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

	virtual AudioFrame *get_current_frame();
	virtual void return_used_frame(AudioFrame *frame);
};
