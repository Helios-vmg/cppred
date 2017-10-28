#pragma once

class CppRedAudioProgram;
enum class AudioResourceId;
enum class SpeciesId;

class CppRedAudioInterface{
	CppRedAudioProgram *program;
	AudioResourceId new_sound_id;
	AudioResourceId last_music_sound_id;
	AudioResourceId after_fade_out_play_this;
public:
	CppRedAudioInterface(CppRedAudioProgram &program);
	CppRedAudioInterface(const CppRedAudioInterface &) = delete;
	CppRedAudioInterface(CppRedAudioInterface &&) = delete;
	void operator=(const CppRedAudioInterface &) = delete;
	void operator=(const CppRedAudioInterface &&) = delete;
	void play_sound(AudioResourceId);
	void play_cry(SpeciesId);
	void pause_music();
	void unpause_music();
};
