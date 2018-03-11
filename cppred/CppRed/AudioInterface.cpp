#include "AudioInterface.h"
#include "AudioProgram.h"
#include "../CodeGeneration/output/audio.h"

namespace CppRed{

AudioInterface::AudioInterface(AudioProgram &program): program(&program){
	this->new_sound_id = AudioResourceId::None;
	this->last_music_sound_id = AudioResourceId::None;
	this->after_fade_out_play_this = AudioResourceId::None;
}

void AudioInterface::play_sound_internal(AudioResourceId id){
	if (this->new_sound_id != AudioResourceId::None)
		for (int i = 4; i < 8; i++)
			this->program->clear_channel(i);
	if (this->program->get_fade_control()){
		if (this->new_sound_id == AudioResourceId::None)
			return;
		this->new_sound_id = AudioResourceId::None;
		if (this->last_music_sound_id == AudioResourceId::Stop){
			this->after_fade_out_play_this = this->last_music_sound_id = id;
			this->program->copy_fade_control();
			return;
		}
		this->program->set_fade_control(0);
	}
	this->new_sound_id = AudioResourceId::None;
	this->program->play_sound(id);
}


void AudioInterface::play_sound(AudioResourceId id){
	auto lock = this->program->acquire_lock();
	this->play_sound_internal(id);
}

void AudioInterface::play_cry(SpeciesId species){
	static const AudioResourceId cries[] = {
		AudioResourceId::SFX_Cry00,
		AudioResourceId::SFX_Cry01,
		AudioResourceId::SFX_Cry02,
		AudioResourceId::SFX_Cry03,
		AudioResourceId::SFX_Cry04,
		AudioResourceId::SFX_Cry05,
		AudioResourceId::SFX_Cry06,
		AudioResourceId::SFX_Cry07,
		AudioResourceId::SFX_Cry08,
		AudioResourceId::SFX_Cry09,
		AudioResourceId::SFX_Cry0A,
		AudioResourceId::SFX_Cry0B,
		AudioResourceId::SFX_Cry0C,
		AudioResourceId::SFX_Cry0D,
		AudioResourceId::SFX_Cry0E,
		AudioResourceId::SFX_Cry0F,
		AudioResourceId::SFX_Cry10,
		AudioResourceId::SFX_Cry11,
		AudioResourceId::SFX_Cry12,
		AudioResourceId::SFX_Cry13,
		AudioResourceId::SFX_Cry14,
		AudioResourceId::SFX_Cry15,
		AudioResourceId::SFX_Cry16,
		AudioResourceId::SFX_Cry17,
		AudioResourceId::SFX_Cry18,
		AudioResourceId::SFX_Cry19,
		AudioResourceId::SFX_Cry1A,
		AudioResourceId::SFX_Cry1B,
		AudioResourceId::SFX_Cry1C,
		AudioResourceId::SFX_Cry1D,
		AudioResourceId::SFX_Cry1E,
		AudioResourceId::SFX_Cry1F,
		AudioResourceId::SFX_Cry20,
		AudioResourceId::SFX_Cry21,
		AudioResourceId::SFX_Cry22,
		AudioResourceId::SFX_Cry23,
		AudioResourceId::SFX_Cry24,
		AudioResourceId::SFX_Cry25,
	};

	auto cry_data = pokemon_by_species_id[(int)species]->cry_data;
	auto id = cries[cry_data.base_cry];
	{
		auto lock = this->program->acquire_lock();
		this->program->set_frequency_modifier(cry_data.pitch);
		this->program->set_tempo_modifier(cry_data.length);
		this->play_sound_internal(id);
	}
	this->program->wait_for_sfx_to_end();
}

void AudioInterface::pause_music(){
	this->program->pause_music();
}

void AudioInterface::unpause_music(){
	this->program->unpause_music();
}

void AudioInterface::wait_for_sfx_to_end(){
	this->program->wait_for_sfx_to_end();
}

}
