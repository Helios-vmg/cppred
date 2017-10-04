#pragma once
#include "Audio.h"
#include "SoundGenerators.h"
#include "PublishingResource.h"

struct Panning{
	bool left = true,
		right = true,
		either = true;
};

class AudioRenderer::ActualRenderer{
	unsigned current_frame_position = 0;
	std::uint64_t frame_no = 0;
	std::uint64_t audio_turned_on_at = 0;
	bool set_audio_turned_on_at_at_next_update = false;
	std::uint64_t current_clock = 0;

	ClockDivider audio_sample_clock,
		frame_sequencer_clock;

	CapacitorFilter filter_left,
		filter_right;
	byte_t NR50 = 0;
	byte_t NR51 = 0;
	bool master_toggle = false;

	Panning stereo_panning[4];

	unsigned left_volume = 0,
		right_volume = 0;

	std::uint64_t speed_counter_a = 0;
	std::uint64_t speed_counter_b = 0;
	std::uint64_t internal_sample_counter = 0;
	StereoSampleFinal last_sample;

	Square1Generator square1;
	Square2Generator square2;
	VoluntaryWaveGenerator wave;
	NoiseGenerator noise;

	static void sample_callback(void *, std::uint64_t);
	static void frame_sequencer_callback(void *, std::uint64_t);
	void sample_callback(std::uint64_t);
	void frame_sequencer_callback(std::uint64_t);
	StereoSampleFinal compute_sample();
	void write_sample(StereoSampleFinal *&buffer);
	void initialize_new_frame();
	StereoSampleIntermediate render_square1(std::uint64_t time);
	StereoSampleIntermediate render_square2(std::uint64_t time);
	StereoSampleIntermediate render_voluntary(std::uint64_t time);
	StereoSampleIntermediate render_noise(std::uint64_t time);
	void length_counter_event();
	void volume_event();
	void sweep_event();
public:
	ActualRenderer();
	void update(double now);

	QueuedPublishingResource<AudioFrame> publishing_frames;
	void set_NR30(byte_t value){
		this->wave.set_register0(value);
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
