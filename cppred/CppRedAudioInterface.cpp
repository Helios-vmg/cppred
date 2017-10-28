#include "CppRedAudioInterface.h"
#include "CppRedAudioProgram.h"
#include "../CodeGeneration/output/audio.h"

CppRedAudioInterface::CppRedAudioInterface(CppRedAudioProgram &program): program(&program){
	this->new_sound_id = AudioResourceId::None;
	this->last_music_sound_id = AudioResourceId::None;
	this->after_fade_out_play_this = AudioResourceId::None;
}

void CppRedAudioInterface::play_sound(AudioResourceId id){
	auto lock = this->program->acquire_lock();
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

void CppRedAudioInterface::play_cry(SpeciesId){
	//TODO
}

void CppRedAudioInterface::pause_music(){
	this->program->pause_music();
}

void CppRedAudioInterface::unpause_music(){
	this->program->unpause_music();
}

void CppRedAudioInterface::wait_for_sfx_to_end(){
	this->program->wait_for_sfx_to_end();
}
