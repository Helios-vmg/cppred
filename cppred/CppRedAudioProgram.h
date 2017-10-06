#pragma once
#include "Audio.h"
#include "CppRedData.h"
#include <set>

enum class AudioCommandType : byte_t{
	Tempo = 0,
	Volume,
	Duty,
	DutyCycle,
	Vibrato,
	TogglePerfectPitch,
	NoteType,
	Rest,
	Octave,
	Note,
	DSpeed,
	Snare,
	MutedSnare,
	UnknownSfx10,
	UnknownSfx20,
	UnknownNoise20,
	ExecuteMusic,
	PitchBend,
	Triangle,
	StereoPanning,
	Cymbal,
	Loop,
	Call,
	Goto,
	IfRed,
	Else,
	EndIf,
	End,
};

struct AudioCommand{
	AudioCommandType type;
	std::uint32_t params[4];
};

struct AudioResource{
	struct Channel{
		std::uint32_t channel;
		std::uint32_t entry_point;
	};
	std::string name;
	Channel channels[8];
	byte_t channel_count;
	byte_t bank;
};

class CppRedAudioProgram : public AudioProgram{
	static const double update_threshold;
	double last_update = -1;
	std::vector<AudioCommand> commands;
	std::vector<AudioResource> resources;
	byte_t mute_audio_and_pause_music = 0;
	byte_t saved_volume = 0xFF;
	int disable_channel_output_when_sfx_ends = 0;
	int music_wave_instrument = 0;
	int sfx_wave_instrument = 0;
	std::uint32_t stereo_panning = 0;
	std::uint32_t music_tempo = 0;
	std::uint32_t sfx_tempo = 0;
	std::uint32_t music_note_delay_counter_fractional_part = 0;
	std::uint32_t sfx_note_delay_counter_fractional_part = 0;
	class Channel{
		std::vector<int> call_stack;
		CppRedAudioProgram *program;
		int channel_no = std::numeric_limits<int>::min();
		AudioResourceId sound_id = AudioResourceId::None;
		int note_delay_counter = 0;
		int vibrato_delay_counter = 0;
		int vibrato_extent = 0;
		int vibrato_counter = 0;
		int vibrato_length = 0;
		int vibrato_depth = 0;
		int vibrato_depth_reload = 0;
		int channel_frequency = 0;
		int vibrato_delay_counter_reload_value = 0;
		int program_counter = 0;
		int note_speed = 0;
		std::uint32_t loop_counter = 1;
		int volume = 0;
		int fade = 0;
		int octave = 0;
		int duty = 0;
		int duty_cycle = 0;
		int pitch_bend_length = 0;
		int pitch_bend_target_frequency = 0;
		bool do_rotate_duty = false;
		bool do_execute_music = false;
		bool do_noise_or_sfx = false;
		bool do_pitch_bend = false;
		bool pitch_bend_decreasing = false;
		bool vibrato_direction = false;
		bool ifred_execute_bit = true;
		bool perfect_pitch = false;

		bool apply_effects(AbstractAudioRenderer &renderer);
		bool play_next_note(AbstractAudioRenderer &renderer);
		void apply_duty_cycle(AbstractAudioRenderer &renderer);
		void apply_pitch_bend(AbstractAudioRenderer &renderer);
		void apply_vibrato(AbstractAudioRenderer &renderer);
		bool continue_execution(AbstractAudioRenderer &renderer);
		bool disable_channel_output(AbstractAudioRenderer &renderer);
		bool disable_channel_output_sub(AbstractAudioRenderer &renderer);
		bool go_back_one_command_if_cry(AbstractAudioRenderer &renderer);

		typedef bool (Channel::*command_function)(const AudioCommand &, AbstractAudioRenderer &, bool &);
#define DECLARE_COMMAND_FUNCTION(x) bool command_##x(const AudioCommand &, AbstractAudioRenderer &renderer, bool &)
		DECLARE_COMMAND_FUNCTION(Tempo);
		DECLARE_COMMAND_FUNCTION(Volume);
		DECLARE_COMMAND_FUNCTION(Duty);
		DECLARE_COMMAND_FUNCTION(DutyCycle);
		DECLARE_COMMAND_FUNCTION(Vibrato);
		DECLARE_COMMAND_FUNCTION(TogglePerfectPitch);
		DECLARE_COMMAND_FUNCTION(NoteType);
		DECLARE_COMMAND_FUNCTION(Rest);
		DECLARE_COMMAND_FUNCTION(Octave);
		DECLARE_COMMAND_FUNCTION(Note);
		DECLARE_COMMAND_FUNCTION(DSpeed);
		DECLARE_COMMAND_FUNCTION(Snare);
		DECLARE_COMMAND_FUNCTION(MutedSnare);
		DECLARE_COMMAND_FUNCTION(UnknownSfx10);
		DECLARE_COMMAND_FUNCTION(UnknownSfx20);
		DECLARE_COMMAND_FUNCTION(UnknownNoise20);
		DECLARE_COMMAND_FUNCTION(ExecuteMusic);
		DECLARE_COMMAND_FUNCTION(PitchBend);
		DECLARE_COMMAND_FUNCTION(Triangle);
		DECLARE_COMMAND_FUNCTION(StereoPanning);
		DECLARE_COMMAND_FUNCTION(Cymbal);
		DECLARE_COMMAND_FUNCTION(Loop);
		DECLARE_COMMAND_FUNCTION(Call);
		DECLARE_COMMAND_FUNCTION(Goto);
		DECLARE_COMMAND_FUNCTION(IfRed);
		DECLARE_COMMAND_FUNCTION(Else);
		DECLARE_COMMAND_FUNCTION(EndIf);
		DECLARE_COMMAND_FUNCTION(End);
		static const command_function command_functions[28];
		static const std::set<AudioResourceId> pokemon_cries;
	public:
		Channel(CppRedAudioProgram &program);
		bool update(AbstractAudioRenderer &renderer);
		//int get_sound_id() const{
		//	return this->sound_id;
		//}
	};
	std::unique_ptr<Channel> channels[8];

	void load_commands();
	void load_resources();
	enum class RegisterId{
		VolumeEnvelope = 2,
		FrequencyLow = 3,
		VolumeEnvelopePlus1 = 4,
	};
	typedef byte_t (*register_function)(AbstractAudioRenderer &, byte_t);
	register_function get_register_pointer(RegisterId);
public:
	CppRedAudioProgram();
	void update(double now, AbstractAudioRenderer &renderer);
};
