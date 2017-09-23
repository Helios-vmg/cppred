#include "CppRed.h"
#include "CppRedAudio.h"

CppRedAudio::CppRedAudio(CppRed &parent): parent(&parent){
}

void CppRedAudio::audio_update_music(){
	const size_t n = this->parent->wram.wChannelSoundIDs.length;
	auto &sc = this->parent->get_sound_controller();
	for (size_t i = 0; i < n; i++){
		auto channel = +this->parent->wram.wChannelSoundIDs[i];
		if (!channel)
			continue;
		auto mute = +this->parent->wram.wMuteAudioAndPauseMusic;
		if (channel >= 4 || !mute){
			this->audio_apply_music_effects(channel);
			continue;
		}
		if (check_flag(mute, wMuteAudioAndPauseMusic_flags::audio_muted))
			continue;
		mute |= wMuteAudioAndPauseMusic_flags::audio_muted;
		this->parent->wram.wMuteAudioAndPauseMusic = mute;
		//Output volume set to zero on both channels.
		sc.set_NR51(0);
		//Disable voluntary wave channel.
		sc.wave.set_register0(0x80);
	}
}

void CppRedAudio::fade_out_audio(){
	const byte_t max_volume = 0x77;

	auto &sc = this->parent->get_sound_controller();
	if (!this->parent->wram.wAudioFadeOutControlCounterRequest){
		if (!this->parent->wram.wMainData.wd72c.get_no_audio_fade_out())
			sc.set_NR50(max_volume);
		return;
	}
	auto counter = +this->parent->wram.wAudioFadeOutCounter;
	if (counter){
		this->parent->wram.wAudioFadeOutCounter = counter - 1;
		return;
	}
	this->parent->wram.wAudioFadeOutCounter = +this->parent->wram.wAudioFadeOutCounterReloadValue;
	
	auto volume = sc.get_NR50();
	if (!volume){
		//Fade-out not yet complete.
		
		//NR50 stores the volumes for both channels as the individual nibbles
		//of a single byte. The following operation decrements both volumes
		//simultaneously.
		volume -= 0x11;

		sc.set_NR50(volume);
		return;
	}

	//Fade-out complete.
	auto old_control = this->parent->wram.wAudioFadeOutControlNextSound.enum_value();
	this->parent->wram.wAudioFadeOutControlNextSound = Sound::None;
	this->parent->wram.wNewSoundID = Sound::Stop;
	this->play_sound(Sound::Stop);
	this->parent->wram.wNewSoundID = old_control;
	this->play_sound(old_control);
}

void CppRedAudio::play_sound(Sound sound){
	if (this->parent->wram.wNewSoundID.value){
		auto i = this->parent->wram.wChannelSoundIDs.begin() + 4;
		std::fill(i, i + 4, 0);
	}
	if (this->parent->wram.wAudioFadeOutControlCounterRequest){
		if (!this->parent->wram.wNewSoundID.value)
			return;
		this->parent->wram.wNewSoundID.value = 0;
		if (this->parent->wram.wLastMusicSoundID.value != 0xFF){
			this->parent->wram.wLastMusicSoundID = sound;

			this->parent->wram.wAudioFadeOutCounter =
			this->parent->wram.wAudioFadeOutCounterReloadValue = this->parent->wram.wAudioFadeOutControlCounterRequest;

			this->parent->wram.wAudioFadeOutControlNextSound = sound;
			return;
		}
		this->parent->wram.wAudioFadeOutControlCounterRequest = 0;
	}
	this->parent->wram.wNewSoundID = Sound::None;
	this->audio_play_sound(sound);
}
