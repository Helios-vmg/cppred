#include "CppRedAudioProgram.h"
#include "CppRedData.h"
#include "utility.h"

const byte_t command_parameter_counts[] = { 1, 2, 1, 1, 3, 0, 3, 1, 1, 2, 1, 2, 2, 1, 4, 3, 0, 2, 2, 1, 2, 2, 1, 1, 0, 0, 0, 0, };

CppRedAudioProgram::CppRedAudioProgram(){
	fill(this->channel_sound_ids, 0);
	fill(this->channel_note_delay_counters, (byte_t)0);
	this->load_commands();
}

void CppRedAudioProgram::load_commands(){
	static_assert(array_length(command_parameter_counts) == (size_t)AudioCommandType::End + 1, "");
	auto buffer = audio_sequence_data;
	size_t offset = 0;
	const size_t size = audio_sequence_data_size;
	this->commands.resize(read_varint(buffer, offset, size));
	for (auto &command : this->commands){
		auto type = read_varint(buffer, offset, size);
		if (type > (std::uint32_t)AudioCommandType::End)
			throw std::runtime_error("CppRedAudioProgram::load_commands(): Invalid data.");
		command.type = (AudioCommandType)type;
		std::fill(command.params, command.params + array_length(command.params), std::numeric_limits<std::uint32_t>::max());
		for (int i = 0; i < (int)command_parameter_counts[type]; i++)
			command.params[i] = read_varint(buffer, offset, size);
	}
	assert(offset == size);
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
