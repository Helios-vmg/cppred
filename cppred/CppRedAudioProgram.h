#pragma once
#include "CppRedData.h"
#include "../common/AudioCommandType.h"
#include "../common/AudioResourceType.h"
#include "pokemon_version.h"
#include "threads.h"
#include <mutex>
#include <memory>
#include <string>

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
	AudioResourceType type;
};

class AudioRenderer;
enum class AudioResourceId;

class CppRedAudioProgram{
	static const double update_threshold;
	double last_update = -1;
	std::vector<AudioCommand> commands;
	std::vector<AudioResource> resources;

	AudioRenderer *renderer;
	PokemonVersion version;
	AudioResourceId sound_id;
	enum class PauseMusicState{
		NotPaused,
		PauseRequested,
		PauseRequestFulfilled,
	};
	PauseMusicState pause_music_state = PauseMusicState::NotPaused;
	byte_t saved_volume = 0xFF;
	int disable_channel_output_when_sfx_ends = 0;
	int music_wave_instrument = 0;
	int sfx_wave_instrument = 0;
	std::uint32_t stereo_panning = 0;
	std::uint32_t music_tempo = 0;
	std::uint32_t sfx_tempo = 0;
	int tempo_modifier = 0;
	int frequency_modifier = 0;
	bool stop_when_sfx_ends = false;
	const AudioResource *current_resource = nullptr;
	std::mutex mutex;
	int fade_out_control = 0;
	int fade_out_counter = 0;
	int fade_out_counter_reload_value = 0;
	Event sfx_finish_event;
	class Channel{
		CppRedAudioProgram *program;
		AudioResourceId sound_id;
		std::vector<int> call_stack;
		int program_counter = 0;
		int bank = 1;
		int channel_no = std::numeric_limits<int>::min();
		byte_t note_delay_counter = 1;
		byte_t note_delay_counter_fractional_part = 0;
		int vibrato_delay_counter = 0;
		int vibrato_extent = 0;
		int vibrato_counter = 0;
		int vibrato_length = 0;
		int channel_frequency = 0;
		int vibrato_delay_counter_reload_value = 0;
		int note_speed = 1;
		std::uint32_t loop_counter = 1;
		int volume = 0;
		int fade = 0;
		int octave = 0;
		int duty = 0;
		int duty_cycle = 0;
		int pitch_bend_length = 0;
		int pitch_bend_target_frequency = 0;
		typedef float pitch_bend_t;
		pitch_bend_t pitch_bend_current_frequency = 0;
		pitch_bend_t pitch_bend_advance = 0;
		bool do_rotate_duty = false;
		bool do_execute_music = false;
		bool do_noise_or_sfx = false;
		bool do_pitch_bend = false;
		bool pitch_bend_decreasing = false;
		bool vibrato_direction = false;
		bool ifred_execute_bit = true;
		bool perfect_pitch = false;

		bool apply_effects();
		bool play_next_note();
		void enable_channel_output();
		void apply_duty_cycle();
		void apply_pitch_bend();
		void apply_duty_and_sound_length();
		void apply_wave_pattern_and_frequency(int frequency);
		void apply_vibrato();
		bool continue_execution();
		bool disable_channel_output();
		bool disable_channel_output_sub();
		bool go_back_one_command_if_cry();
		void note_length(std::uint32_t length_parameter, std::uint32_t note_parameter);
		void note_pitch(std::uint32_t note_parameter);
		void disable_this_hardware_channel();
		void set_delay_counters(std::uint32_t length_parameter);
		int init_pitch_bend_variables(int frequency);
		void set_sfx_tempo();

		typedef bool (Channel::*command_function)(const AudioCommand &, bool &);
#define DECLARE_COMMAND_FUNCTION(x) bool command_##x(const AudioCommand &, bool &)
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
		DECLARE_COMMAND_FUNCTION(NoiseInstrument);
		DECLARE_COMMAND_FUNCTION(UnknownSfx10);
		DECLARE_COMMAND_FUNCTION(UnknownSfx20);
		DECLARE_COMMAND_FUNCTION(UnknownNoise20);
		DECLARE_COMMAND_FUNCTION(ExecuteMusic);
		DECLARE_COMMAND_FUNCTION(PitchBend);
		DECLARE_COMMAND_FUNCTION(StereoPanning);
		DECLARE_COMMAND_FUNCTION(Loop);
		DECLARE_COMMAND_FUNCTION(Call);
		DECLARE_COMMAND_FUNCTION(Goto);
		DECLARE_COMMAND_FUNCTION(IfRed);
		DECLARE_COMMAND_FUNCTION(Else);
		DECLARE_COMMAND_FUNCTION(EndIf);
		DECLARE_COMMAND_FUNCTION(End);
		bool unknown20(const AudioCommand &, bool &dont_stop_this_channel, bool noise);
		static const command_function command_functions[28];
	public:
		Channel(CppRedAudioProgram &program, int channel_no, AudioResourceId resource_id, int entry_point, int bank);
		bool update();
		//int get_sound_id() const{
		//	return this->sound_id;
		//}
		bool is_cry();
		void reset(AudioResourceId resource_id, int entry_point, int bank);
		AudioResourceId get_sound_id() const{
			return this->sound_id;
		}
		int get_program_counter() const{
			return this->program_counter;
		}
		void set_program_counter(int pc){
			this->program_counter = pc;
		}
	};
	std::unique_ptr<Channel> channels[8];

	void load_commands();
	void load_resources();
	bool is_cry();
	enum class RegisterId{
		DutySoundLength = 1,
		VolumeEnvelope = 2,
		FrequencyLow = 3,
		FrequencyHigh = 4,
	};
	typedef byte_t (*register_function)(AudioRenderer &, int);
	register_function get_register_pointer(RegisterId, int channel_no);
	void set_register(RegisterId reg, int channel_no, byte_t value){
		this->get_register_pointer(reg, channel_no)(*this->renderer, value);
	}
	byte_t get_register(RegisterId reg, int channel_no){
		return this->get_register_pointer(reg, channel_no)(*this->renderer, -1);
	}
	void perform_update();
	void update_channel(int);
	void compute_fade_out();
	bool is_sfx_playing();
public:
	CppRedAudioProgram(AudioRenderer &renderer, PokemonVersion);
	void update(double now);
	void play_sound(AudioResourceId);
	void pause_music();
	void unpause_music();
	void clear_channel(int channel);
	std::vector<std::string> get_resource_strings();
	std::unique_lock<std::mutex> acquire_lock();
	int get_fade_control() const{
		return this->fade_out_control;
	}
	void set_fade_control(int f){
		this->fade_out_control = f;
	}
	void copy_fade_control();
	void wait_for_sfx_to_end();
};
