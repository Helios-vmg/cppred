#include "BlarggRenderer.h"

namespace GbRegisters{
const unsigned NR10 = 0xFF10;
const unsigned NR11 = 0xFF11;
const unsigned NR12 = 0xFF12;
const unsigned NR13 = 0xFF13;
const unsigned NR14 = 0xFF14;

const unsigned NR21 = 0xFF16;
const unsigned NR22 = 0xFF17;
const unsigned NR23 = 0xFF18;
const unsigned NR24 = 0xFF19;

const unsigned NR30 = 0xFF1A;
const unsigned NR31 = 0xFF1B;
const unsigned NR32 = 0xFF1C;
const unsigned NR33 = 0xFF1D;
const unsigned NR34 = 0xFF1E;

const unsigned NR41 = 0xFF20;
const unsigned NR42 = 0xFF21;
const unsigned NR43 = 0xFF22;
const unsigned NR44 = 0xFF23;

const unsigned NR50 = 0xFF24;
const unsigned NR51 = 0xFF25;
const unsigned NR52 = 0xFF26;

const unsigned WAVE = 0xFF30;
}

BlarggRenderer::BlarggRenderer(): apu(new Basic_Gb_Apu){
	if (this->apu->set_sample_rate(44100))
		throw std::runtime_error("BlarggRenderer::BlarggRenderer(): Unable to set rample rate.");
}

void BlarggRenderer::update(double now){
	this->clock = (decltype(this->clock))(now * gb_cpu_frequency);
	if (this->last_update < 0){
		this->last_update = now;
		return;
	}
	if (now - this->last_update < 1.0 / 60.0)
		return;
	this->apu->end_frame();
	auto n = this->apu->samples_avail();
	std::vector<blip_sample_t> temp(n);
	this->apu->read_samples(&temp[0], n);
	StereoSampleFinal *buffer = this->publishing_frames.get_private_resource()->buffer;
	for (decltype(n) i = 0; i < n; i += 2){
		StereoSampleFinal sample;
		sample.left = temp[i + 0];
		sample.right = temp[i + 1];
		this->write_sample(buffer, sample);
	}
	this->last_update = now;
}

#define DEFINE_FORWARDING_REGISTER_SET(reg)                 \
void BlarggRenderer::set_NR##reg(byte_t value){             \
	this->apu->write_register(this->clock, GbRegisters::NR##reg, value); \
}

#define DEFINE_FORWARDING_REGISTER_GET(reg)                \
byte_t BlarggRenderer::get_NR##reg() const{                \
	return this->apu->read_register(this->clock, GbRegisters::NR##reg); \
}

DEFINE_FORWARDING_REGISTER_SET(10)
DEFINE_FORWARDING_REGISTER_SET(11)
DEFINE_FORWARDING_REGISTER_SET(12)
DEFINE_FORWARDING_REGISTER_SET(13)
DEFINE_FORWARDING_REGISTER_SET(14)

DEFINE_FORWARDING_REGISTER_SET(21)
DEFINE_FORWARDING_REGISTER_SET(22)
DEFINE_FORWARDING_REGISTER_SET(23)
DEFINE_FORWARDING_REGISTER_SET(24)

DEFINE_FORWARDING_REGISTER_SET(30)
DEFINE_FORWARDING_REGISTER_SET(31)
DEFINE_FORWARDING_REGISTER_SET(32)
DEFINE_FORWARDING_REGISTER_SET(33)
DEFINE_FORWARDING_REGISTER_SET(34)

DEFINE_FORWARDING_REGISTER_SET(41)
DEFINE_FORWARDING_REGISTER_SET(42)
DEFINE_FORWARDING_REGISTER_SET(43)
DEFINE_FORWARDING_REGISTER_SET(44)

DEFINE_FORWARDING_REGISTER_SET(50)
DEFINE_FORWARDING_REGISTER_SET(51)
DEFINE_FORWARDING_REGISTER_SET(52)

DEFINE_FORWARDING_REGISTER_GET(11)
DEFINE_FORWARDING_REGISTER_GET(12)
DEFINE_FORWARDING_REGISTER_GET(13)
DEFINE_FORWARDING_REGISTER_GET(14)

DEFINE_FORWARDING_REGISTER_GET(21)
DEFINE_FORWARDING_REGISTER_GET(22)
DEFINE_FORWARDING_REGISTER_GET(23)
DEFINE_FORWARDING_REGISTER_GET(24)

DEFINE_FORWARDING_REGISTER_GET(31)
DEFINE_FORWARDING_REGISTER_GET(32)
DEFINE_FORWARDING_REGISTER_GET(33)
DEFINE_FORWARDING_REGISTER_GET(34)

DEFINE_FORWARDING_REGISTER_GET(41)
DEFINE_FORWARDING_REGISTER_GET(42)
DEFINE_FORWARDING_REGISTER_GET(43)
DEFINE_FORWARDING_REGISTER_GET(44)

DEFINE_FORWARDING_REGISTER_GET(50)
DEFINE_FORWARDING_REGISTER_GET(51)
DEFINE_FORWARDING_REGISTER_GET(52)

void BlarggRenderer::copy_voluntary_wave(const void *buffer){
	for (int i = 16; i--;)
		this->apu->write_register(this->clock, GbRegisters::WAVE + i, ((const byte_t *)buffer)[i]);
}
