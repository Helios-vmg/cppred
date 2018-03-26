#include "stdafx.h"
#include "SoundGenerators.h"
#include "utility.h"

const int int16_max = (1 << 15) - 1;

const byte_t Square2Generator::duties[4] = {
	0x80,
	0x81,
	0xE1,
	0x7E,
};

ClockDivider::ClockDivider(){
	this->src_frequency_power = 0;
	this->dst_frequency = 0;
	this->last_update = 0;
}

#ifdef USE_STD_FUNCTION
ClockDivider::ClockDivider(unsigned src_frequency_power, std::uint64_t dst_frequency, callback_t &&callback){
	this->configure(src_frequency_power, dst_frequency, std::move(callback));
}
#else
ClockDivider::ClockDivider(unsigned src_frequency_power, std::uint64_t dst_frequency, callback_t callback, void *user_data){
	this->configure(src_frequency_power, dst_frequency, callback, user_data);
}
#endif

#ifdef USE_STD_FUNCTION
void ClockDivider::configure(unsigned src_frequency_power, std::uint64_t dst_frequency, callback_t &&callback){
	this->callback = callback;
#else
void ClockDivider::configure(unsigned src_frequency_power, std::uint64_t dst_frequency, callback_t callback, void *user_data){
	this->callback = callback;
	this->user_data = user_data;
#endif
	this->src_frequency_power = src_frequency_power;
	this->dst_frequency = dst_frequency;
	this->reset();
}

void ClockDivider::update(std::uint64_t source_clock){
	if (!this->src_frequency_power)
		return;

	auto time = source_clock * this->dst_frequency;
	time >>= this->src_frequency_power;
	if (this->last_update == std::numeric_limits<std::uint64_t>::max())
		this->last_update = time - 1;
	else if (time == this->last_update)
		return;
	auto dst_clock = this->last_update + 1;
	for (; dst_clock <= time; dst_clock++)
#ifdef USE_STD_FUNCTION
		this->callback(dst_clock);
#else
		this->callback(this->user_data, dst_clock);
#endif
	this->last_update = time;
}

void ClockDivider::reset(){
	this->last_update = std::numeric_limits<std::uint64_t>::max();
}

WaveformGenerator::WaveformGenerator(){
	fill<byte_t>(this->registers, 0);
}

void WaveformGenerator::trigger_event(){
}

void EnvelopedGenerator::trigger_event(){
	WaveformGenerator::trigger_event();
	this->envelope_time = this->envelope_period;
	this->load_volume_from_register();
	if (!this->sound_length)
		this->sound_length = 64;
}

void Square2Generator::trigger_event(){
	EnvelopedGenerator::trigger_event();
	this->cycle_position = 0;
	this->reset_references();
}

void Square1Generator::trigger_event(){
	Square2Generator::trigger_event();
	this->shadow_frequency = this->frequency;
	if (!!this->sweep_period & !!this->sweep_shift){
		this->sweep_event(true);
	}
}

unsigned Square2Generator::get_period(){
	if (!this->period)
		this->period = (2048 - this->frequency) * 4;
	return this->period;
}

template <unsigned Shift>
void FrequenciedGenerator::advance_cycle(std::uint64_t time){
	bool und1 = this->reference_time == this->undefined_reference_time;
	bool und2 = this->reference_cycle_position == this->undefined_reference_cycle_position;
	if (und1 & und2){
		this->reference_time = time;
		this->reference_cycle_position = this->cycle_position;
	}else{
		auto delta = time - this->reference_time;
		this->cycle_position = this->reference_cycle_position;
		const auto mult = (std::uint64_t)gb_cpu_frequency << Shift;
		auto div = sampling_frequency * this->get_period();
		this->cycle_position += (unsigned)(delta * mult / div) & 0xFFFF;
		this->cycle_position &= 0xFFFF;
	}
}

void Square2Generator::update_state_before_render(std::uint64_t time){
	this->advance_cycle<13>(time);
}

intermediate_audio_type Square2Generator::render(std::uint64_t time) const{
	if (!this->enabled())
		return 0;

	bool bit = !!(this->duties[this->selected_duty] & ::bit(this->cycle_position >> 13));
	return this->render_from_bit(bit);
}

intermediate_audio_type EnvelopedGenerator::render_from_bit(bool signal) const{
#ifdef USE_FLOAT_AUDIO
	auto y = (signal * 2 - 1) * this->volume;
	return (intermediate_audio_type)y * (1.f / 15.f);
#else
	auto y = (signal * 2 - 1) * this->volume;
	return y * int16_max / 15;
#endif
}

bool WaveformGenerator::enabled() const{
	return this->length_counter_has_not_finished();
}

bool WaveformGenerator::length_counter_has_not_finished() const{
	return !this->length_enable | !!this->sound_length;
}

bool EnvelopedGenerator::enabled() const{
	return WaveformGenerator::enabled() & !!this->volume;
}

bool Square2Generator::enabled() const{
	//Note: if frequency value > 2041, sound frequency > 20 kHz
	return EnvelopedGenerator::enabled() & (this->frequency > 0) & (this->frequency <= 2041);
}

bool Square1Generator::enabled() const{
	return Square2Generator::enabled() & (this->shadow_frequency != this->audio_disabled_by_sweep);
}

void FrequenciedGenerator::write_register3_frequency(byte_t value){
	auto old = this->frequency;
	this->frequency &= ~(unsigned)0xFF;
	this->frequency |= value;
	this->frequency_change(old);
}

void FrequenciedGenerator::write_register4_frequency(byte_t value){
	unsigned freq = value & 0x07;
	freq <<= 8;
	auto old = this->frequency;
	this->frequency &= ~(unsigned)0x0700;
	this->frequency |= freq;
	this->frequency_change(old);
}

void WaveformGenerator::set_register1(byte_t value){
	this->registers[1] = value;
	this->sound_length = this->shadow_sound_length = 64 - (value & 0x3F);
}

void Square2Generator::set_register1(byte_t value){
	WaveformGenerator::set_register1(value);
	this->selected_duty = value >> 6;
}

void EnvelopedGenerator::load_volume_from_register(){
	this->volume = this->registers[2] >> 4;
}

void EnvelopedGenerator::set_register2(byte_t value){
	this->registers[2] = value;

	this->load_volume_from_register();
	this->envelope_sign = value & bit(3) ? 1 : -1;
	this->envelope_period = value & 0x07;
}

void Square2Generator::set_register3(byte_t value){
	this->registers[3] = value;

	this->write_register3_frequency(value);
	this->sound_length = this->shadow_sound_length;
}

void WaveformGenerator::set_register4(byte_t value){
	this->registers[4] = value;

	bool trigger = !!(value & bit(7));
	this->length_enable = !!(value & bit(6));

	this->sound_length = this->shadow_sound_length;

	if (trigger)
		this->trigger_event();
}

void Square2Generator::set_register4(byte_t value){
	this->write_register4_frequency(value);
	WaveformGenerator::set_register4(value);
}

byte_t Square2Generator::get_register1() const{
	return this->registers[1] & 0x3F;
}

byte_t EnvelopedGenerator::get_register2() const{
	return this->registers[2];
}

byte_t Square2Generator::get_register3() const{
	return 0xFF;
}

byte_t WaveformGenerator::get_register4() const{
	return this->registers[4] | 0xBF;
}

void Square1Generator::set_register0(byte_t value){
	this->registers[0] = value;

	this->sweep_period = (value & BITMAP(01110000)) >> 4;
	this->sweep_sign = value & bit(3) ? -1 : 1;
	this->sweep_shift = value & BITMAP(00000111);
}

byte_t Square1Generator::get_register0() const{
	return this->registers[0] | 0x80;
}

void FrequenciedGenerator::frequency_change(unsigned old_frequency){
	if (this->frequency == old_frequency)
		return;
	this->period = 0;
	this->reset_references();
}

void FrequenciedGenerator::reset_references(){
	this->reference_time = this->undefined_reference_time;
	this->reference_cycle_position = this->undefined_reference_cycle_position;
}

void Square1Generator::sweep_event(bool force){
	if (!force){
		if (!this->sweep_period | !this->sweep_shift | !this->sweep_time)
			return;
		this->sweep_time--;
		if (this->sweep_time)
			return;
	}

	this->sweep_time = this->sweep_period;
	auto old = this->frequency;
	this->frequency = this->shadow_frequency;
	this->frequency_change(old);
	this->shadow_frequency += this->sweep_sign * (this->shadow_frequency >> this->sweep_shift);

	if (this->shadow_frequency < 0)
		this->shadow_frequency = 0;
	else if (this->shadow_frequency >= 2048){
		this->sweep_time = 0;
		this->shadow_frequency = this->audio_disabled_by_sweep;
	}
}

void EnvelopedGenerator::volume_event(){
	if (!this->envelope_period | !this->envelope_time)
		return;
	this->envelope_time--;
	if (this->envelope_time)
		return;

	this->envelope_time = this->envelope_period;
	this->volume += this->envelope_sign;
	//Saturate value into range [0; 15]. Note: this only works if this->envelope_sign == +/-1.
	this->volume -= this->volume >> 4;
}

void WaveformGenerator::length_counter_event(){
	if (this->sound_length)
		this->sound_length--;
}

intermediate_audio_type CapacitorFilter::update(intermediate_audio_type in){
	auto ret = in - this->state;
#ifdef USE_FLOAT_AUDIO
	const float multiplier = 0.999958f; // use 0.998943 for MGB&CGB
	//Note: multiplier2 = multiplier ^ (2^22 / 44100)
	const float multiplier2 = 0.996013f;
	this->state = in - ret * multiplier2;
	return ret;
#else
	//Note: 15/16 + 61355/1048576 is approximately equal to 0.996013.
	const int multiplier1 = 15;
	const int multiplier2 = 61355;
	const int shift1 = 4;
	const int shift2 = 20;
	this->state = in - (((ret * multiplier1) >> shift1) + ((ret * multiplier2) >> shift2));
	return ret;
#endif
}

void NoiseGenerator::set_register3(byte_t value){
	EnvelopedGenerator::set_register3(value);
	this->width_mode = 14 - (value & bit(3));

	unsigned clock_shift = value >> 4;
	unsigned divisor_code = value & 0x07;
	unsigned frequency;
	if (!divisor_code)
		frequency = 1 << 20;
	else
		frequency = (1 << 19) / divisor_code;
	frequency >>= clock_shift + 1;

	this->noise_scheduler.configure(
		gb_cpu_frequency_power,
		frequency,
#ifdef USE_STD_FUNCTION
		[this](std::uint64_t)
		{
			this->noise_update_event();
		}
#else
		NoiseGenerator::noise_update_event,
		this
#endif
	);
}

intermediate_audio_type NoiseGenerator::render(std::uint64_t time) const{
	if (!this->enabled())
		return 0;

	return this->render_from_bit(this->output);
}

void NoiseGenerator::update_state_before_render(std::uint64_t time){
	this->noise_scheduler.update(time);
}

void NoiseGenerator::noise_update_event(void *This, std::uint64_t){
	((NoiseGenerator *)This)->noise_update_event();
}

void NoiseGenerator::noise_update_event(){
	auto output = this->noise_register;
	this->noise_register >>= 1;
	output ^= this->noise_register;
	output &= 1;
	this->noise_register &= ~bit(this->width_mode);
	this->noise_register |= output << this->width_mode;
	this->noise_register &= 0x7FFF;
	this->output ^= !!output;
}

void NoiseGenerator::trigger_event(){
	EnvelopedGenerator::trigger_event();
	this->noise_register |= ~this->noise_register;
}

VoluntaryWaveGenerator::VoluntaryWaveGenerator(){
	fill<byte_t>(this->wave_buffer, 0);
}

void VoluntaryWaveGenerator::set_register0(byte_t value){
	this->registers[0] = value;
	this->dac_power = !!(value & bit(7));
}

void VoluntaryWaveGenerator::set_register1(byte_t value){
	WaveformGenerator::set_register1(value);
	this->sound_length = this->shadow_sound_length = 256 - value;
}

void VoluntaryWaveGenerator::set_register2(byte_t value){
	this->registers[2] = value;

	unsigned volume_code = (value >> 5) & 3;
	this->volume_shift = (volume_code + 4) % 5;
}

void VoluntaryWaveGenerator::set_register3(byte_t value){
	this->registers[3] = value;
	this->write_register3_frequency(value);
}

void VoluntaryWaveGenerator::set_register4(byte_t value){
	this->write_register4_frequency(value);
	WaveformGenerator::set_register4(value);
}

void VoluntaryWaveGenerator::set_wave_table(unsigned position, byte_t value){
	position %= array_length(this->wave_buffer) / 2;
	position *= 2;
	this->wave_buffer[position + 0] = value >> 4;
	this->wave_buffer[position + 1] = value & 0x0F;
}

byte_t VoluntaryWaveGenerator::get_register0() const{
	return this->registers[0] | 0x7F;
}

byte_t VoluntaryWaveGenerator::get_register1() const{
	return 0xFF;
}

byte_t VoluntaryWaveGenerator::get_register2() const{
	return this->registers[2] | 0x9F;
}

byte_t VoluntaryWaveGenerator::get_register3() const{
	return 0xFF;
}

void VoluntaryWaveGenerator::update_state_before_render(std::uint64_t time){
	this->advance_cycle<11>(time);
	this->sample_register = this->wave_buffer[this->cycle_position >> 11];
}

intermediate_audio_type VoluntaryWaveGenerator::render(std::uint64_t time) const{
	if (!this->enabled())
		return 0;

#ifdef USE_FLOAT_AUDIO
	return (this->sample_register >> this->volume_shift) * (2.f / 15.f) - 1;
#else
	auto ret = this->sample_register >> this->volume_shift;
	ret *= 2 * int16_max;
	ret /= 15;
	ret -= int16_max;
	return ret;
#endif
}

unsigned VoluntaryWaveGenerator::get_period(){
	if (!this->period)
		this->period = (2048 - this->frequency) * 2;
	return this->period;
}

bool VoluntaryWaveGenerator::enabled() const{
	return WaveformGenerator::enabled() & (this->volume_shift != 4) & !!this->frequency & this->dac_power;
}

void VoluntaryWaveGenerator::trigger_event(){
	WaveformGenerator::trigger_event();
	this->cycle_position = 0;
	this->reset_references();
	if (!this->sound_length)
		this->sound_length = 256;
	this->dac_power = true;
}
