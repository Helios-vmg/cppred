#include "CppRedAudioProgram.h"
#include "utility.h"

CppRedAudioProgram::CppRedAudioProgram(){
	fill(this->channel_sound_ids, 0);
	fill(this->channel_note_delay_counters, (byte_t)0);
}

void CppRedAudioProgram::update(double now, AbstractAudioRenderer &renderer){
	for (int channel = 0; channel < (int)array_length(this->channel_sound_ids); channel++){
		auto sound_id = this->channel_sound_ids[channel];
		if (!sound_id)
			continue;
		if (channel >= 4 || !this->mute_audio_and_pause_music){
			this->apply_effects(channel, now, renderer);
			continue;
		}
		if (this->mute_audio_and_pause_music & (1 << 7))
			continue;
		this->mute_audio_and_pause_music |= 1 << 7;
		renderer.set_nr51(0);
		renderer.set_nr30(0);
		renderer.set_nr30(0x80);
	}
}

void CppRedAudioProgram::apply_effects(int channel, double now, AbstractAudioRenderer &renderer){
	
}
