#pragma once
#include "AudioRenderer.h"
#include "Gb_Snd_Emu-0.1.4/Basic_Gb_Apu.h"
#include <memory>

class BlarggRenderer : public AudioRenderer{
	std::unique_ptr<Basic_Gb_Apu> apu;
	double last_update = -1;
	std::uint64_t frame_no = 0;
	QueuedPublishingResource<AudioFrame> publishing_frames;
	std::int64_t clock;
public:
	BlarggRenderer();
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
	byte_t get_NR50() const override;
	byte_t get_NR51() const override;
	byte_t get_NR52() const override;
	void copy_voluntary_wave(const void *buffer) override;
};
