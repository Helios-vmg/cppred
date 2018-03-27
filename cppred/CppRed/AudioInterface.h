#pragma once

enum class AudioResourceId;
enum class SpeciesId;

namespace CppRed{

class AudioProgramInterface;

class AudioInterface{
	AudioProgramInterface *program;
	AudioResourceId new_sound_id;
	AudioResourceId last_music_sound_id;
	AudioResourceId after_fade_out_play_this;
	void play_sound_internal(AudioResourceId);
public:
	AudioInterface(AudioProgramInterface &program);
	AudioInterface(const AudioInterface &) = delete;
	AudioInterface(AudioInterface &&) = delete;
	void operator=(const AudioInterface &) = delete;
	void operator=(const AudioInterface &&) = delete;
	void play_sound(AudioResourceId);
	void play_cry(SpeciesId);
	void pause_music();
	void unpause_music();
	void wait_for_sfx_to_end();
	void fade_out_music_to_silence(double duration);
	void fade_out_music_then_change_tracks(AudioResourceId, double duration);
	void stop_sfx();
};

}
