#pragma once
#include "SoundGenerators.h"
#include "PublishingResource.h"
#include "AudioData.h"
#include "AudioDevice.h"
#ifndef HAVE_PCH
#include <fstream>
#include <SDL_hints.h>
#endif

//#define OUTPUT_AUDIO_TO_FILE

struct Panning{
	bool left = true,
		right = true,
		either = true;
};

class AudioRenderer{
	std::mutex mutex;
	std::uint64_t expected_frame = 0;
protected:
	AbstractAudioDevice *device;
	bool active = false;
public:
	AudioRenderer(AbstractAudioDevice &device);
	virtual ~AudioRenderer();
	virtual void update(double now) = 0;
	virtual void start(){}
	typedef std::pair<AudioFrame *, AudioRenderer *> frame_t;
	virtual AudioFrame *get_current_frame() = 0;
	virtual void return_used_frame(AudioFrame *frame) = 0;
	virtual frame_t get_current_frame_with_object(){
		return {this->get_current_frame(), this};
	}
	static void return_used_frame(const frame_t &frame){
		frame.second->return_used_frame(frame.first);
	}
	void write_data_to_device(Uint8 *stream, int len, StereoSampleFinal *spillover_buffer, size_t &spillover_buffer_size);
	virtual void set_active(bool active){
		this->active = active;
	}
};

class GbAudioRenderer : public AudioRenderer{
public:
	GbAudioRenderer(AbstractAudioDevice &device): AudioRenderer(device){}
	virtual ~GbAudioRenderer(){}
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
};

class TwoWayMixer : public AudioRenderer, public AbstractAudioDevice{
	std::mutex mutex;
	std::unique_ptr<GbAudioRenderer> low_priority_renderer;
	std::unique_ptr<GbAudioRenderer> high_priority_renderer;
	moodycamel::ReaderWriterQueue<AudioRenderer::frame_t> queue;
	int volume_divisor = 1;

	AudioFrame *get_current_frame() override;
	void return_used_frame(AudioFrame *frame) override;
	frame_t get_current_frame_with_object() override;
public:
	TwoWayMixer(AudioDevice &device);
	~TwoWayMixer();
	void set_renderers(std::unique_ptr<GbAudioRenderer> &&low_priority_renderer, std::unique_ptr<GbAudioRenderer> &&high_priority_renderer);
	void start() override;
	void update(double now) override;
	void set_renderer(AudioRenderer &) override{}
	void clear_renderer() override{}
	void add_volume_divisor(int d){
		this->volume_divisor *= d;
	}
	void remove_volume_divisor(int d){
		this->volume_divisor /= d;
	}
	GbAudioRenderer &get_low_priority_renderer(){
		return *this->low_priority_renderer;
	}
	GbAudioRenderer &get_high_priority_renderer(){
		return *this->high_priority_renderer;
	}
};
