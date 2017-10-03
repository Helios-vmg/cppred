#include "SoundGenerators.h"

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
