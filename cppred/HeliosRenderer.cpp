#include "HeliosRenderer.h"
#include "utility.h"
#include <sstream>

#define CHANNEL_SELECTION 0xF
#define CHANNEL1 (1 << 0)
#define CHANNEL2 (1 << 1)
#define CHANNEL3 (1 << 2)
#define CHANNEL4 (1 << 3)

HeliosRenderer::HeliosRenderer():
#ifdef USE_STD_FUNCTION
		audio_sample_clock(gb_cpu_frequency_power, sampling_frequency, [this](std::uint64_t n){ this->sample_callback(n); }),
		frame_sequencer_clock(gb_cpu_frequency_power, 512, [this](std::uint64_t n){ this->frame_sequencer_callback(n); })
#else
		audio_sample_clock(gb_cpu_frequency_power, sampling_frequency, sample_callback, this),
		frame_sequencer_clock(gb_cpu_frequency_power, 512, frame_sequencer_callback, this)
#endif
{
#ifdef OUTPUT_AUDIO_TO_FILE
	this->output_file.reset(new std::ofstream("output-0.raw", std::ios::binary));
	bool abort = false;
	if (*this->output_file){
		int i = 0;
		for (auto &file : this->output_files_by_channel){
			std::stringstream stream;
			stream << "output-" << i + 1 << ".raw";
			file.reset(new std::ofstream(stream.str().c_str(), std::ios::binary));
			if (!*file){
				abort = true;
				break;
			}
			this->output_buffers_by_channel[i].reset(new AudioFrame);
			i++;
		}
	} else
		abort = true;
	if (abort){
		this->output_file.reset();
		for (auto &file : this->output_files_by_channel)
			file.reset();
		for (auto &buffer : this->output_buffers_by_channel)
			buffer.reset();
	}
#endif
}

void HeliosRenderer::update(double now){
	this->current_clock = cast_round_u64(now * gb_cpu_frequency);
	if (this->set_audio_turned_on_at_at_next_update){
		this->audio_turned_on_at = this->current_clock;
		this->set_audio_turned_on_at_at_next_update = false;
	}
	auto t = this->current_clock - this->audio_turned_on_at;
	std::uint64_t i, loops;
	if (t < this->last_simulated_time){
		i = t - t % 4;
		loops = 1;
	}else{
		i = this->last_simulated_time;
		loops = (t - i) / 4;
	}

	for (std::uint64_t j = 0; j < loops; j++, i += 4){
		this->noise.update_state_before_render(i);
		this->frame_sequencer_clock.update(i);
		this->audio_sample_clock.update(i);
	}
	this->last_simulated_time = i;
}

void HeliosRenderer::sample_callback(void *This, std::uint64_t sample_no){
	((HeliosRenderer *)This)->sample_callback(sample_no);
}

void HeliosRenderer::frame_sequencer_callback(void *This, std::uint64_t clock){
	((HeliosRenderer *)This)->frame_sequencer_callback(clock);
}

void HeliosRenderer::sample_callback(std::uint64_t sample_no){
	StereoSampleFinal *buffer = this->publishing_frames.get_private_resource()->buffer;
	this->last_sample = this->compute_sample();
	this->write_sample(buffer);
}

void HeliosRenderer::frame_sequencer_callback(std::uint64_t clock){
	if (!this->master_toggle)
		return;

	if (!(clock % 2))
		this->length_counter_event();
	if (clock % 8 == 7)
		this->volume_event();
	if (clock % 4 == 2)
		this->sweep_event();
}

StereoSampleFinal HeliosRenderer::compute_sample(){
	std::uint64_t sample_no = this->internal_sample_counter++;
	if (!this->master_toggle){
		StereoSampleFinal ret;
		ret.left = ret.right = 0;
		return ret;
	}

	StereoSampleIntermediate channels[4];
	channels[0] = this->render_square1(sample_no);
	channels[1] = this->render_square2(sample_no);
	channels[2] = this->render_voluntary(sample_no);
	channels[3] = this->render_noise(sample_no);

	StereoSampleIntermediate sample;
	sample.left = sample.right = 0;

	for (int i = 4; i--;){
#ifdef OUTPUT_AUDIO_TO_FILE
		if (this->output_buffers_by_channel[i])
			this->output_buffers_by_channel[i]->buffer[this->current_frame_position] = convert(channels[i]);
#endif
		sample += channels[i];
	}
	sample /= 4;
	sample.left = this->filter_left.update(sample.left);
	sample.right = this->filter_right.update(sample.right);

	sample.left *= this->left_volume;
	sample.right *= this->right_volume;
	sample /= 15;

	return convert(sample);
}

void HeliosRenderer::write_sample(StereoSampleFinal *&buffer){
	buffer[this->current_frame_position++] = this->last_sample;
	if (this->current_frame_position >= AudioFrame::length){
#ifdef OUTPUT_AUDIO_TO_FILE
		if (this->output_file)
			this->output_file->write((const char *)buffer, AudioFrame::length * sizeof(StereoSampleFinal));
		for (int i = 4; i--;){
			auto &buffer2 = this->output_buffers_by_channel[i]->buffer;
			if (this->output_files_by_channel[i])
				this->output_files_by_channel[i]->write((const char *)buffer2, sizeof(buffer2));
		}
#endif
		this->current_frame_position = 0;
		this->publishing_frames.publish();
		this->initialize_new_frame();
		buffer = this->publishing_frames.get_private_resource()->buffer;
	}
}

void HeliosRenderer::initialize_new_frame(){
	auto frame = this->publishing_frames.get_private_resource();
	frame->frame_no = this->frame_no++;
	auto &buffer = frame->buffer;
	memset(buffer, 0, sizeof(buffer));
}

template <typename T1, typename T2>
StereoSampleIntermediate compute_channel_panning_and_silence(const T1 &generator, std::uint64_t time, const T2 &pan){
	StereoSampleIntermediate ret;
	if (!!pan.either){
		auto value = generator.render(time);

		ret.left = value * !!pan.left;
		ret.right = value * !!pan.right;
	}else
		ret.left = ret.right = 0;

	return ret;
}

StereoSampleIntermediate HeliosRenderer::render_square1(std::uint64_t time){
	this->square1.update_state_before_render(time);
#if CHANNEL_SELECTION & CHANNEL1
	return compute_channel_panning_and_silence(this->square1, time, this->stereo_panning[0]);
#else
	StereoSampleIntermediate ret;
	ret.left = ret.right = 0;
	return ret;
#endif
}

StereoSampleIntermediate HeliosRenderer::render_square2(std::uint64_t time){
	this->square2.update_state_before_render(time);
#if CHANNEL_SELECTION & CHANNEL2
	return compute_channel_panning_and_silence(this->square2, time, this->stereo_panning[1]);
#else
	StereoSampleIntermediate ret;
	ret.left = ret.right = 0;
	return ret;
#endif
}

StereoSampleIntermediate HeliosRenderer::render_voluntary(std::uint64_t time){
	this->wave.update_state_before_render(time);
#if CHANNEL_SELECTION & CHANNEL3
	return compute_channel_panning_and_silence(this->wave, time, this->stereo_panning[2]);
#else
	StereoSampleIntermediate ret;
	ret.left = ret.right = 0;
	return ret;
#endif
}

StereoSampleIntermediate HeliosRenderer::render_noise(std::uint64_t time){
#if CHANNEL_SELECTION & CHANNEL4
	return compute_channel_panning_and_silence(this->noise, time, this->stereo_panning[3]);
#else
	StereoSampleIntermediate ret;
	ret.left = ret.right = 0;
	return ret;
#endif
}

void HeliosRenderer::length_counter_event(){
	this->square1.length_counter_event();
	this->square2.length_counter_event();
	this->noise.length_counter_event();
}

void HeliosRenderer::volume_event(){
	this->square1.volume_event();
	this->square2.volume_event();
	this->noise.volume_event();
}

void HeliosRenderer::sweep_event(){
	this->square1.sweep_event();
}

void HeliosRenderer::set_NR10(byte_t value){
	this->square1.set_register0(value);
}

void HeliosRenderer::set_NR11(byte_t value){
	this->square1.set_register1(value);
}

void HeliosRenderer::set_NR12(byte_t value){
	this->square1.set_register2(value);
}

void HeliosRenderer::set_NR13(byte_t value){
	this->square1.set_register3(value);
}

void HeliosRenderer::set_NR14(byte_t value){
	this->square1.set_register4(value);
}

void HeliosRenderer::set_NR21(byte_t value){
	this->square2.set_register1(value);
}

void HeliosRenderer::set_NR22(byte_t value){
	this->square2.set_register2(value);
}

void HeliosRenderer::set_NR23(byte_t value){
	this->square2.set_register3(value);
}

void HeliosRenderer::set_NR24(byte_t value){
	this->square2.set_register4(value);
}

void HeliosRenderer::set_NR30(byte_t value){
	this->wave.set_register0(value);
}

void HeliosRenderer::set_NR31(byte_t value){
	this->wave.set_register1(value);
}
void HeliosRenderer::set_NR32(byte_t value){
	this->wave.set_register2(value);
}

void HeliosRenderer::set_NR33(byte_t value){
	this->wave.set_register3(value);
}

void HeliosRenderer::set_NR34(byte_t value){
	this->wave.set_register4(value);
}

void HeliosRenderer::set_NR41(byte_t value){
	this->noise.set_register1(value);
}

void HeliosRenderer::set_NR42(byte_t value){
	this->noise.set_register2(value);
}

void HeliosRenderer::set_NR43(byte_t value){
	this->noise.set_register3(value);
}

void HeliosRenderer::set_NR44(byte_t value){
	this->noise.set_register4(value);
}

void HeliosRenderer::set_NR50(byte_t value){
	this->NR50 = value;

	this->left_volume = (value >> 4) & 0x07;
	this->right_volume = value & 0x07;
}

void HeliosRenderer::set_NR51(byte_t value){
	this->NR51 = value;

	for (int channel = 0; channel < 4; channel++){
		auto &pan = this->stereo_panning[channel];
		pan.right = !!(value & bit(channel));
		pan.left = !!(value & bit(channel + 4));
		pan.either = pan.right | pan.left;
	}
}

void HeliosRenderer::set_NR52(byte_t value){
	auto mt = this->master_toggle;
	this->master_toggle = !!(value & bit(7));
	if (this->master_toggle & !mt){
		this->set_audio_turned_on_at_at_next_update = true;
		this->frame_sequencer_clock.reset();
		this->audio_sample_clock.reset();
		this->internal_sample_counter = 0;
		this->speed_counter_a = 0;
		this->speed_counter_b = 0;
		this->last_sample *= 0;
	}
}

byte_t HeliosRenderer::get_NR11() const{
	return this->square1.get_register1();
}

byte_t HeliosRenderer::get_NR12() const{
	return this->square1.get_register2();
}

byte_t HeliosRenderer::get_NR13() const{
	return this->square1.get_register3();
}

byte_t HeliosRenderer::get_NR14() const{
	return this->square1.get_register4();
}

byte_t HeliosRenderer::get_NR21() const{
	return this->square2.get_register1();
}

byte_t HeliosRenderer::get_NR22() const{
	return this->square2.get_register2();
}

byte_t HeliosRenderer::get_NR23() const{
	return this->square2.get_register3();
}

byte_t HeliosRenderer::get_NR24() const{
	return this->square2.get_register4();
}

byte_t HeliosRenderer::get_NR31() const{
	return this->wave.get_register1();
}

byte_t HeliosRenderer::get_NR32() const{
	return this->wave.get_register2();
}

byte_t HeliosRenderer::get_NR33() const{
	return this->wave.get_register3();
}

byte_t HeliosRenderer::get_NR34() const{
	return this->wave.get_register4();
}

byte_t HeliosRenderer::get_NR41() const{
	return this->noise.get_register1();
}

byte_t HeliosRenderer::get_NR42() const{
	return this->noise.get_register2();
}

byte_t HeliosRenderer::get_NR43() const{
	return this->noise.get_register3();
}

byte_t HeliosRenderer::get_NR44() const{
	return this->noise.get_register4();
}

byte_t HeliosRenderer::get_NR52() const{
	auto ret = (byte_t)this->master_toggle << 7;
	ret |= 0x70;
	ret |= (byte_t)this->square1.length_counter_has_not_finished() << 0;
	ret |= (byte_t)this->square2.length_counter_has_not_finished() << 1;
	ret |= bit(2);
	ret |= (byte_t)this->noise.length_counter_has_not_finished() << 3;
	return ret;
}

void HeliosRenderer::copy_voluntary_wave(const void *buffer){
	for (int i = 16; i--;)
		this->wave.set_wave_table(i, ((const byte_t *)buffer)[i]);
}

AudioFrame *HeliosRenderer::get_current_frame(){
	return this->publishing_frames.get_public_resource();
}

void HeliosRenderer::return_used_frame(AudioFrame *frame){
	this->publishing_frames.return_resource(frame);
}
