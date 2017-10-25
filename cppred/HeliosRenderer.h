#pragma once
#include "AudioRenderer.h"

class HeliosRenderer : public AudioRenderer{
	unsigned current_frame_position = 0;
	std::uint64_t frame_no = 0;
	std::uint64_t audio_turned_on_at = 0;
	bool set_audio_turned_on_at_at_next_update = false;
	std::uint64_t current_clock = 0;
	std::uint64_t last_simulated_time = std::numeric_limits<std::uint64_t>::max();

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
#ifdef OUTPUT_AUDIO_TO_FILE
	std::unique_ptr<std::ofstream> output_file;
	std::unique_ptr<std::ofstream> output_files_by_channel[4];
	std::unique_ptr<AudioFrame> output_buffers_by_channel[4];
#endif
	Square1Generator square1;
	Square2Generator square2;
	VoluntaryWaveGenerator wave;
	NoiseGenerator noise;
	QueuedPublishingResource<AudioFrame> publishing_frames;

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
	HeliosRenderer();
	void update(double now) override;

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
	byte_t get_NR11() const override;
	byte_t get_NR12() const override;
	byte_t get_NR13() const override;
	byte_t get_NR14() const override;
	byte_t get_NR21() const override;
	byte_t get_NR22() const override;
	byte_t get_NR23() const override;
	byte_t get_NR24() const override;
	byte_t get_NR31() const override;
	byte_t get_NR32() const override;
	byte_t get_NR33() const override;
	byte_t get_NR34() const override;
	byte_t get_NR41() const override;
	byte_t get_NR42() const override;
	byte_t get_NR43() const override;
	byte_t get_NR44() const override;
	byte_t get_NR50() const override{
		return this->NR50;
	}
	byte_t get_NR51() const override{
		return this->NR51;
	}
	byte_t get_NR52() const override;
	void copy_voluntary_wave(const void *buffer) override;

	AudioFrame *get_current_frame() override;
	void return_used_frame(AudioFrame *frame) override;
};
