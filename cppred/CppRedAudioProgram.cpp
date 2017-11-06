#include "CppRedAudioProgram.h"
#include "CppRedData.h"
#include "utility.h"
#include "AudioRenderer.h"
#include "../common/calculate_frequency.h"
#include "../CodeGeneration/output/audio.h"
#include <set>
#include <sstream>

const byte_t command_parameter_counts[] = {
	1, //tempo
	2, //volume
	1, //duty
	1, //duty_cycle
	3, //vibrato
	0, //toggle_perfect_pitch
	3, //note_type
	1, //rest
	1, //octave
	2, //note
	1, //dspeed
	2, //noise_instrument
	1, //unknown_sfx_10
	3, //unknown_sfx_20
	3, //unknown_noise_20
	0, //execute_music
	2, //pitch_bend
	1, //stereo_panning
	2, //loop
	1, //call
	1, //goto
	0, //ifred
	0, //else
	0, //endif
	0, //end
};

#define DECLARE_COMMAND_FUNCTION_IN_ARRAY(x) &CppRedAudioProgram::Channel::command_##x

const CppRedAudioProgram::Channel::command_function CppRedAudioProgram::Channel::command_functions[28] = {
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Tempo),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Volume),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Duty),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(DutyCycle),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Vibrato),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(TogglePerfectPitch),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(NoteType),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Rest),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Octave),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Note),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(DSpeed),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(NoiseInstrument),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(UnknownSfx10),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(UnknownSfx20),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(UnknownNoise20),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(ExecuteMusic),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(PitchBend),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(StereoPanning),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Loop),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Call),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Goto),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(IfRed),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(Else),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(EndIf),
	DECLARE_COMMAND_FUNCTION_IN_ARRAY(End),
};

#define DEFINE_AC_STRUCT0(name)
#define DEFINE_AC_STRUCT1(name, p1)                  \
	struct name##AudioCommand{                       \
		std::uint32_t p1;                            \
		name##AudioCommand(const AudioCommand &cmd): \
			p1(cmd.params[0]){}                      \
	}
#define DEFINE_AC_STRUCT2(name, p1, p2)              \
	struct name##AudioCommand{                       \
		std::uint32_t p1;                            \
		std::uint32_t p2;                            \
		name##AudioCommand(const AudioCommand &cmd): \
			p1(cmd.params[0]),                       \
			p2(cmd.params[1]){}                      \
	}
#define DEFINE_AC_STRUCT3(name, p1, p2, p3)          \
	struct name##AudioCommand{                       \
		std::uint32_t p1;                            \
		std::uint32_t p2;                            \
		std::uint32_t p3;                            \
		name##AudioCommand(const AudioCommand &cmd): \
			p1(cmd.params[0]),                       \
			p2(cmd.params[1]),                       \
			p3(cmd.params[2]){}                      \
	}
#define DEFINE_AC_STRUCT4(name, p1, p2, p3, p4)      \
	struct name##AudioCommand{                       \
		std::uint32_t p1;                            \
		std::uint32_t p2;                            \
		std::uint32_t p3;                            \
		std::uint32_t p4;                            \
		name##AudioCommand(const AudioCommand &cmd): \
			p1(cmd.params[0]),                       \
			p2(cmd.params[1]),                       \
			p3(cmd.params[2]),                       \
			p4(cmd.params[3]){}                      \
	}

DEFINE_AC_STRUCT1(Tempo, tempo);
DEFINE_AC_STRUCT2(Volume, vol1, vol2);
DEFINE_AC_STRUCT1(Duty, duty);
DEFINE_AC_STRUCT1(DutyCycle, duty);
DEFINE_AC_STRUCT3(Vibrato, delay, rate, depth);
DEFINE_AC_STRUCT0(TogglePerfectPitch);
DEFINE_AC_STRUCT3(NoteType, speed, volume, fade);
DEFINE_AC_STRUCT1(Rest, length);
DEFINE_AC_STRUCT1(Octave, octave);
DEFINE_AC_STRUCT2(Note, pitch, length);
DEFINE_AC_STRUCT1(DSpeed, speed);
DEFINE_AC_STRUCT2(NoiseInstrument, pitch, length);
DEFINE_AC_STRUCT1(UnknownSfx10, nr10);
DEFINE_AC_STRUCT3(UnknownSfx20, length, envelope, frequency);
DEFINE_AC_STRUCT3(UnknownNoise20, length, envelope, frequency);
DEFINE_AC_STRUCT0(ExecuteMusic);
DEFINE_AC_STRUCT2(PitchBend, length, frequency);
DEFINE_AC_STRUCT1(StereoPanning, value);
DEFINE_AC_STRUCT2(Loop, times, dst);
DEFINE_AC_STRUCT1(Call, dst);
DEFINE_AC_STRUCT1(Goto, dst);
DEFINE_AC_STRUCT0(IfRed);
DEFINE_AC_STRUCT0(Else);
DEFINE_AC_STRUCT0(EndIf);
DEFINE_AC_STRUCT0(End);

static const byte_t disable_channel_masks[] = {
	BITMAP(11101110),
	BITMAP(11011101),
	BITMAP(10111011),
	BITMAP(01110111),
};

static const byte_t enable_channel_masks[] = {
	BITMAP(00010001),
	BITMAP(00100010),
	BITMAP(01000100),
	BITMAP(10001000),
};

typedef std::array<byte_t, 16> instrument_data_t;

static const instrument_data_t instruments[] = {
	{ 0x02, 0x46, 0x8A, 0xCE, 0xFF, 0xFE, 0xED, 0xDC, 0xCB, 0xA9, 0x87, 0x65, 0x44, 0x33, 0x22, 0x11, },
	{ 0x02, 0x46, 0x8A, 0xCE, 0xEF, 0xFF, 0xFE, 0xEE, 0xDD, 0xCB, 0xA9, 0x87, 0x65, 0x43, 0x22, 0x11, },
	{ 0x13, 0x69, 0xBD, 0xEE, 0xEE, 0xFF, 0xFF, 0xED, 0xDE, 0xFF, 0xFF, 0xEE, 0xEE, 0xDB, 0x96, 0x31, },
	{ 0x02, 0x46, 0x8A, 0xCD, 0xEF, 0xFE, 0xDE, 0xFF, 0xEE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, },
	{ 0x01, 0x23, 0x45, 0x67, 0x8A, 0xCD, 0xEE, 0xF7, 0x7F, 0xEE, 0xDC, 0xA8, 0x76, 0x54, 0x32, 0x10, },
	{ 0x21, 0xE2, 0x33, 0x28, 0xE1, 0x22, 0xFF, 0xEA, 0x10, 0x14, 0xDC, 0x10, 0xE3, 0x41, 0x51, 0x73, },
	{ 0xEC, 0x02, 0x20, 0x91, 0xC0, 0x07, 0x20, 0x81, 0xD0, 0x07, 0x20, 0x91, 0xC0, 0x07, 0x2C, 0xA1, },
	{ 0x21, 0xE2, 0x33, 0x28, 0xE1, 0x22, 0xFF, 0x22, 0xF7, 0x24, 0x22, 0xF7, 0x34, 0x24, 0xF7, 0x44, },
};

static const instrument_data_t * const instruments_bank_1[] = {
	instruments + 0,
	instruments + 1,
	instruments + 2,
	instruments + 3,
	instruments + 4,
	instruments + 5,
};

static const instrument_data_t * const instruments_bank_2[] = {
	instruments + 0,
	instruments + 1,
	instruments + 2,
	instruments + 3,
	instruments + 4,
	instruments + 6,
};

static const instrument_data_t * const instruments_bank_3[] = {
	instruments + 0,
	instruments + 1,
	instruments + 2,
	instruments + 3,
	instruments + 4,
	instruments + 7,
};

static const instrument_data_t * const * const instruments_by_bank[] = {
	instruments_bank_1,
	instruments_bank_2,
	instruments_bank_3,
};

CppRedAudioProgram::CppRedAudioProgram(AudioRenderer &renderer, PokemonVersion version): renderer(&renderer), version(version){
	this->load_commands();
	this->load_resources();
}

void CppRedAudioProgram::load_commands(){
	static_assert(array_length(command_parameter_counts) == (size_t)AudioCommandType::End + 1, "Error: command_parameter_counts must have as many elements as there are command types!");
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

void CppRedAudioProgram::load_resources(){
	auto buffer = audio_header_data;
	size_t offset = 0;
	const size_t size = audio_header_data_size;
	this->resources.resize(read_varint(buffer, offset, size) + 1);
	assert(this->resources.size() == (size_t)AudioResourceId::Stop);
	bool skip = true;
	for (auto &resource : this->resources){
		if (skip){
			skip = false;
			continue;
		}
		resource.name = read_string(buffer, offset, size);
		resource.bank = (byte_t)read_varint(buffer, offset, size);
		resource.type = (AudioResourceType)read_varint(buffer, offset, size);
		resource.channel_count = (byte_t)read_varint(buffer, offset, size);
		for (byte_t i = 0; i < resource.channel_count; i++){
			resource.channels[i].entry_point = read_varint(buffer, offset, size);
			resource.channels[i].channel = read_varint(buffer, offset, size);
		}
	}
}

const double CppRedAudioProgram::update_threshold = 4389.0 / 262144.0;

void CppRedAudioProgram::update(double now){
	LOCK_MUTEX(this->mutex);
	auto delta = now - this->last_update;
	if (this->last_update < 0){
		this->last_update = now;
		return;
	}
	if (delta < update_threshold)
		return;
	int n = (int)(delta * (1.0 / update_threshold));
	
	for (int i = n; i--;)
		this->perform_update();
	this->last_update = now - (delta - n * update_threshold);
}

void CppRedAudioProgram::update_channel(int i){
	auto &c = this->channels[i];
	if (!c)
		return;
	if (!c->update()){
		c.reset();
		if (i >= 4 && !this->is_sfx_playing())
			this->sfx_finish_event.signal();
	}
}

void CppRedAudioProgram::compute_fade_out(){
	//TODO
	this->renderer->set_NR50(0x77);
}

void CppRedAudioProgram::perform_update(){
	this->compute_fade_out();
	int c = 0;
	if (this->pause_music_state == PauseMusicState::NotPaused){
		for (; c < 4; c++)
			this->update_channel(c);
	}else
		c = 4;
	for (; c < array_length(this->channels); c++)
		this->update_channel(c);
	if (this->pause_music_state == PauseMusicState::PauseRequested){
		this->renderer->set_NR51(0);
		this->renderer->set_NR30(0);
		this->renderer->set_NR30(0x80);
		this->pause_music_state = PauseMusicState::PauseRequestFulfilled;
	}
}

void CppRedAudioProgram::pause_music(){
	LOCK_MUTEX(this->mutex);
	this->pause_music_state = PauseMusicState::PauseRequested;
}

void CppRedAudioProgram::unpause_music(){
	LOCK_MUTEX(this->mutex);
	this->pause_music_state = PauseMusicState::NotPaused;
}

bool CppRedAudioProgram::Channel::update(){
	return this->apply_effects();
}

bool CppRedAudioProgram::Channel::apply_effects(){
	if (this->note_delay_counter == 1)
		return this->play_next_note();
	this->note_delay_counter--;
	if (this->channel_no < 4 && this->program->channels[4 + this->channel_no])
		return true;
	if (this->do_rotate_duty)
		this->apply_duty_cycle();
	if (!this->do_execute_music && this->do_noise_or_sfx)
		return true;
	if (this->do_pitch_bend){
		this->apply_pitch_bend();
		return true;
	}
	this->apply_vibrato();
	return true;
}

void CppRedAudioProgram::Channel::apply_vibrato(){
	if (this->vibrato_delay_counter){
		this->vibrato_delay_counter--;
		return;
	}
	if (!this->vibrato_extent)
		return;
	if (this->vibrato_counter){
		this->vibrato_counter--;
		return;
	}
	this->vibrato_counter = this->vibrato_length;
	int diff;
	if (this->vibrato_direction){
		this->vibrato_direction = false;
		diff = this->channel_frequency - this->vibrato_extent % 16;
		if (diff < 0)
			diff = 0;
	}else{
		this->vibrato_direction = true;
		diff = this->channel_frequency % 256 + this->vibrato_extent / 16;
		if (diff > 255)
			diff = 255;
	}
	this->program->set_register(RegisterId::FrequencyLow, this->channel_no, (byte_t)diff);
}

bool CppRedAudioProgram::Channel::play_next_note(){
	this->vibrato_delay_counter = this->vibrato_delay_counter_reload_value;
	this->do_pitch_bend = false;
	this->pitch_bend_decreasing = false;
	return this->continue_execution();
}

bool CppRedAudioProgram::Channel::continue_execution(){
	bool ret = true;
	while (true){
		auto &command = this->program->commands[this->program_counter++];
		this->program_counter %= this->program->commands.size();
		if (!this->ifred_execute_bit)
			continue;
		if (!(this->*this->command_functions[(int)command.type])(command, ret))
			break;
	}
	return ret;
}

#define DEFINE_COMMAND_FUNCTION(x) bool CppRedAudioProgram::Channel::command_##x(const AudioCommand &command_, bool &dont_stop_this_channel)
//#define LOG_COMMAND_EXECUTION

DEFINE_COMMAND_FUNCTION(Tempo){
	TempoAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "tempo " << command.tempo << std::endl;
#endif
	int offset;
	if (this->channel_no < 4){
		this->program->music_tempo = command.tempo;
		offset = 0;
	}else{
		this->program->sfx_tempo = command.tempo;
		offset = 4;
	}
	for (int i = 0; i < 4; i++){
		auto &channel = this->program->channels[i + offset];
		if (channel)
			channel->note_delay_counter_fractional_part = 0;
	}
	return true;
}

DEFINE_COMMAND_FUNCTION(Volume){
	VolumeAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "volume " << command.vol1 << " " << command.vol2 << std::endl;
#endif
	this->program->renderer->set_NR50((byte_t)((command.vol1 & 0x0F) | ((command.vol2 & 0x0F) << 4)));
	return true;
}

DEFINE_COMMAND_FUNCTION(Duty){
	DutyAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "duty " << command.duty << std::endl;
#endif
	this->duty = command.duty;
	return true;
}

DEFINE_COMMAND_FUNCTION(DutyCycle){
	DutyCycleAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "duty_cycle " << command.duty << std::endl;
#endif
	this->duty_cycle = command.duty;
	this->duty = command.duty & 0xC0;
	this->do_rotate_duty = true;
	return true;
}

DEFINE_COMMAND_FUNCTION(Vibrato){
	VibratoAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "vibrato " << command.delay << " " << command.rate << " " << command.depth << std::endl;
#endif
	this->vibrato_delay_counter_reload_value = this->vibrato_delay_counter = command.delay;
	this->vibrato_extent = command.rate / 2;
	this->vibrato_extent += (this->vibrato_extent + command.rate % 2) << 4;
	this->vibrato_length = this->vibrato_counter = command.depth;
	return true;
}

DEFINE_COMMAND_FUNCTION(TogglePerfectPitch){
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "toggle_perfect_pitch\n";
#endif
	this->perfect_pitch = !this->perfect_pitch;
	return true;
}

DEFINE_COMMAND_FUNCTION(NoteType){
	NoteTypeAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "note_type " << command.speed << " " << command.volume << " " << command.fade << std::endl;
#endif
	this->note_speed = command.speed;

	int *dst = nullptr;
	if (this->channel_no == 2)
		dst = &this->program->music_wave_instrument;
	else if (this->channel_no == 6)
		dst = &this->program->sfx_wave_instrument;
	if (dst){
		*dst = command.fade;
		this->volume = (command.volume * 2) % 16;
	}else{
		this->volume = command.volume;
		this->fade = command.fade;
	}
	return true;
}

DEFINE_COMMAND_FUNCTION(Rest){
	RestAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "rest " << command.length << std::endl;
#endif
	this->set_delay_counters(command.length);
	if (this->channel_no < 4 && this->program->channels[4 + this->channel_no])
		return true;
	if (this->channel_no % 4 == 2){
		this->disable_this_hardware_channel();
	}else{
		this->program->set_register(RegisterId::VolumeEnvelope, this->channel_no, 0x08);
		//Restart sound on current channel.
		this->program->set_register(RegisterId::FrequencyHigh, this->channel_no, 0x80);
	}
	return false;
}

DEFINE_COMMAND_FUNCTION(Octave){
	OctaveAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "octave " << command.octave << std::endl;
#endif
	this->octave = command.octave;
	return true;
}

DEFINE_COMMAND_FUNCTION(Note){
	NoteAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "note " << command.pitch << " " << command.length << std::endl;
#endif
	this->note_length(command.length, command.pitch);
	return false;
}

DEFINE_COMMAND_FUNCTION(DSpeed){
	DSpeedAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "dspeed " << command.speed << std::endl;
#endif
	this->note_speed = command.speed;
	return true;
}

DEFINE_COMMAND_FUNCTION(NoiseInstrument){
	NoiseInstrumentAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "noise_instrument " << command.pitch << " " << command.length << std::endl;
#endif
	if (!this->program->stop_when_sfx_ends){
		static const AudioResourceId noises[] = {
			AudioResourceId::SFX_Snare1,
			AudioResourceId::SFX_Snare2,
			AudioResourceId::SFX_Snare3,
			AudioResourceId::SFX_Snare4,
			AudioResourceId::SFX_Snare5,
			AudioResourceId::SFX_Triangle1,
			AudioResourceId::SFX_Triangle2,
			AudioResourceId::SFX_Snare6,
			AudioResourceId::SFX_Snare7,
			AudioResourceId::SFX_Snare8,
			AudioResourceId::SFX_Snare9,
			AudioResourceId::SFX_Cymbal1,
			AudioResourceId::SFX_Cymbal2,
			AudioResourceId::SFX_Cymbal3,
			AudioResourceId::SFX_Muted_Snare1,
			AudioResourceId::SFX_Triangle3,
			AudioResourceId::SFX_Muted_Snare2,
			AudioResourceId::SFX_Muted_Snare3,
			AudioResourceId::SFX_Muted_Snare4,
		};
		auto pitch = command.pitch - 1;
		if (pitch < 0 || pitch >= array_length(noises)){
			std::stringstream stream;
			stream << "Bad noise instrument command. Attempt to call invalid noise " << pitch;
			throw std::runtime_error(stream.str());
		}
		this->program->play_sound(noises[pitch]);
	}
	this->note_length(command.length, command.pitch);
	return false;
}

DEFINE_COMMAND_FUNCTION(UnknownSfx10){
#ifndef LOG_COMMAND_EXECUTION
	if (this->channel_no < 4 || this->do_execute_music)
		return true;
	UnknownSfx10AudioCommand command(command_);
#else
	UnknownSfx10AudioCommand command(command_);
	std::cout << "unknown_sfx_10 " << command.nr10 << std::endl;
	if (this->channel_no < 4 || this->do_execute_music)
		return true;
#endif
	this->program->renderer->set_NR10((byte_t)command.nr10);
	return true;
}

DEFINE_COMMAND_FUNCTION(UnknownSfx20){
	return this->unknown20(command_, dont_stop_this_channel, false);
}

DEFINE_COMMAND_FUNCTION(UnknownNoise20){
	return this->unknown20(command_, dont_stop_this_channel, true);
}

bool CppRedAudioProgram::Channel::unknown20(const AudioCommand &command_, bool &dont_stop_this_channel, bool noise){
#ifndef LOG_COMMAND_EXECUTION
	if (this->channel_no < 3 || this->do_execute_music)
		return true;
	UnknownSfx20AudioCommand command(command_);
#else
	UnknownSfx20AudioCommand command(command_);
	std::cout << (noise ? "unknown_noise_20 " : "unknown_sfx_20 ") << command.length << " " << command.envelope << " " << command.frequency << std::endl;
	if (this->channel_no < 3 || this->do_execute_music)
		return true;
#endif
	this->note_length(command.length, 2);
	this->program->set_register(RegisterId::DutySoundLength, this->channel_no, this->duty | this->note_delay_counter);
	this->program->set_register(RegisterId::VolumeEnvelope, this->channel_no, command.envelope);
	this->apply_duty_and_sound_length();
	this->enable_channel_output();
	this->apply_wave_pattern_and_frequency(command.frequency);
	return false;
}

DEFINE_COMMAND_FUNCTION(ExecuteMusic){
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "execute_music\n";
#endif
	this->do_execute_music = true;
	return true;
}

DEFINE_COMMAND_FUNCTION(PitchBend){
	PitchBendAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "pitch_bend " << command.length << " " << command.frequency << std::endl;
#endif
	this->pitch_bend_length = command.length;
	this->pitch_bend_target_frequency = command.frequency;
	this->do_pitch_bend = true;
	return true;
}

DEFINE_COMMAND_FUNCTION(StereoPanning){
	StereoPanningAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "stereo_panning " << command.value << std::endl;
#endif
	this->program->stereo_panning = command.value;
	return true;
}

DEFINE_COMMAND_FUNCTION(Loop){
	LoopAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "loop " << command.times << " " << command.dst << std::endl;
#endif
	if (!command.times){
		this->program_counter = command.dst;
		return true;
	}
	if (this->loop_counter < command.times){
		this->loop_counter++;
		this->program_counter = command.dst;
		return true;
	}
	this->loop_counter = 1;
	return true;
}

DEFINE_COMMAND_FUNCTION(Call){
	CallAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "call " << command.dst << std::endl;
#endif
	if (this->call_stack.size())
		std::cout << "Assumption in audio program might have been invalidated.\n";
	this->call_stack.push_back(this->program_counter);
	this->program_counter = command.dst;
	return true;
}

DEFINE_COMMAND_FUNCTION(Goto){
	GotoAudioCommand command(command_);
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "goto " << command.dst << std::endl;
#endif
	this->program_counter = command.dst;
	return true;
}

DEFINE_COMMAND_FUNCTION(IfRed){
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "ifred\n";
#endif
	this->ifred_execute_bit = this->program->version == PokemonVersion::Red;
	return true;
}

DEFINE_COMMAND_FUNCTION(Else){
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "else\n";
#endif
	this->ifred_execute_bit = !ifred_execute_bit;
	return true;
}

DEFINE_COMMAND_FUNCTION(EndIf){
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "endif\n";
#endif
	this->ifred_execute_bit = true;
	return true;
}

DEFINE_COMMAND_FUNCTION(End){
#ifdef LOG_COMMAND_EXECUTION
	std::cout << "end\n";
#endif
	if (this->call_stack.size()){
		this->program_counter = this->call_stack.back();
		this->call_stack.pop_back();
		return true;
	}

	if (this->channel_no < 3){
		dont_stop_this_channel = !this->disable_channel_output();
		return false;
	}
	this->do_noise_or_sfx = false;
	this->do_execute_music = false;
	if (this->channel_no == 6){
		this->program->renderer->set_NR30(0);
		this->program->renderer->set_NR30(0x80);
		if (this->program->disable_channel_output_when_sfx_ends){
			this->program->disable_channel_output_when_sfx_ends = 0;
			dont_stop_this_channel = !this->disable_channel_output();
			return false;
		}
	}
	dont_stop_this_channel = !this->disable_channel_output_sub();
	return false;
}

bool CppRedAudioProgram::Channel::disable_channel_output(){
	this->disable_this_hardware_channel();
	return this->disable_channel_output_sub();
}

bool CppRedAudioProgram::Channel::disable_channel_output_sub(){
	if (this->program->channels[4] && this->program->channels[4]->is_cry()){
		if (this->channel_no == 4 && this->go_back_one_command_if_cry())
			return false;
		this->program->renderer->set_NR50(this->program->saved_volume);
		this->program->saved_volume = 0;
	}
	return true;
}

bool CppRedAudioProgram::Channel::go_back_one_command_if_cry(){
	if (!this->program->is_cry())
		return false;
	this->program_counter--;
	return true;
}

void CppRedAudioProgram::Channel::note_length(std::uint32_t length_parameter, std::uint32_t note_parameter){
	this->set_delay_counters(length_parameter);
	if (this->do_execute_music | !this->do_noise_or_sfx)
		this->note_pitch(note_parameter);
}

void CppRedAudioProgram::Channel::set_delay_counters(std::uint32_t length_parameter){
	auto length = this->note_speed * length_parameter;
	std::uint32_t tempo;
	if (this->channel_no < 4)
		tempo = this->program->music_tempo;
	else if (this->channel_no != 7){
		this->set_sfx_tempo();
		tempo = this->program->sfx_tempo;
	}else
		tempo = 0x0100;
	auto i = tempo * length + this->note_delay_counter_fractional_part;
	this->note_delay_counter_fractional_part = i & 0xFF;
	this->note_delay_counter = (i >> 8) & 0xFF;
}

void CppRedAudioProgram::Channel::disable_this_hardware_channel(){
	auto r = this->program->renderer->get_NR51();
	r &= disable_channel_masks[this->channel_no % 4];
	this->program->renderer->set_NR51(r);
}

void CppRedAudioProgram::Channel::note_pitch(std::uint32_t note_parameter){
	int frequency = calculate_frequency(note_parameter, this->octave);
	if (this->do_pitch_bend)
		frequency = this->init_pitch_bend_variables(frequency);
	if (this->channel_no < 4 && this->program->channels[4 + this->channel_no])
		return;
	this->program->set_register(RegisterId::VolumeEnvelope, this->channel_no, ((this->volume << 4) & 0xF0)|(this->fade & 0x0F));
	this->apply_duty_and_sound_length();
	this->enable_channel_output();
	if (this->perfect_pitch)
		frequency++;
	this->channel_frequency = frequency;
	this->apply_wave_pattern_and_frequency(frequency);
}

static int implementation2(int delta, int length){
	int quotient;
	int remainder;
	if (delta >= 0x100){
		delta -= 0x100;
		quotient = delta / length + 1;
		remainder = delta % length + 0x100 - length;
	}else{
		quotient = 1;
		remainder = euclidean_modulo(delta - length, 0x100);
	}

	return (quotient << 8) | remainder;
}

int CppRedAudioProgram::Channel::init_pitch_bend_variables(int frequency){
	this->pitch_bend_current_frequency = (pitch_bend_t)frequency;
	this->pitch_bend_length = this->note_delay_counter - this->pitch_bend_length;
	if (this->pitch_bend_length <= 0)
		this->pitch_bend_length = 1;
	int delta = this->pitch_bend_target_frequency - frequency;
	if (delta >= 0)
		this->pitch_bend_decreasing = false;
	else
		this->pitch_bend_decreasing = true;
	this->pitch_bend_advance = (pitch_bend_t)delta / this->pitch_bend_length;

	return implementation2(delta, this->pitch_bend_length);
}

void CppRedAudioProgram::Channel::apply_duty_and_sound_length(){
	auto temp = this->note_delay_counter;
	if (this->channel_no % 4 != 2)
		temp = (temp & BITMAP(00111111)) | this->duty;
	this->program->set_register(RegisterId::DutySoundLength, this->channel_no, temp);
}

void CppRedAudioProgram::Channel::enable_channel_output(){
	auto nr51 = this->program->renderer->get_NR51();
	byte_t value;
	if (this->channel_no == 7 || this->channel_no < 4 && !this->program->channels[4 + this->channel_no]){
		value = nr51 & disable_channel_masks[this->channel_no % 4];
		value |= this->program->stereo_panning & enable_channel_masks[this->channel_no % 4];
	}else
		value = nr51 | enable_channel_masks[this->channel_no % 4];
	this->program->renderer->set_NR51(value);
}

void CppRedAudioProgram::Channel::set_sfx_tempo(){
	int tempo;
	if (this->program->is_cry()){
		tempo = this->program->tempo_modifier + 0x80;
		int d = tempo >= 256;
		tempo &= 0xFF;
		tempo = tempo | (tempo << 8);
	}else{
		tempo = 0x100;
	}
	this->program->sfx_tempo = tempo;
}

void CppRedAudioProgram::Channel::apply_duty_cycle(){
	this->duty_cycle &= 0xFF;
	this->duty_cycle = (this->duty_cycle << 2) | (this->duty_cycle >> 6);
	this->duty_cycle &= 0xFF;
	auto reg = this->program->get_register(RegisterId::DutySoundLength, this->channel_no);
	this->program->set_register(RegisterId::DutySoundLength, this->channel_no, (this->duty_cycle & BITMAP(11000000)) | (reg & BITMAP(00111111)));
}

void CppRedAudioProgram::Channel::apply_pitch_bend(){
	bool reached_target = false;
	auto new_frequency = this->pitch_bend_current_frequency + this->pitch_bend_advance;
	auto whole_part = cast_round(new_frequency);

	if (!this->pitch_bend_decreasing)
		reached_target = whole_part >= this->pitch_bend_target_frequency;
	else
		reached_target = whole_part <= this->pitch_bend_target_frequency;

	if (!reached_target){
		this->pitch_bend_current_frequency = new_frequency;
		this->program->set_register(RegisterId::FrequencyLow, this->channel_no, whole_part & 0xFF);
		this->program->set_register(RegisterId::FrequencyHigh, this->channel_no, (whole_part >> 8) & 0xFF);
		return;
	}
	this->do_pitch_bend = false;
	this->pitch_bend_decreasing = false;
}

void CppRedAudioProgram::Channel::apply_wave_pattern_and_frequency(int frequency){
	if (this->channel_no % 4 == 2){
		auto instrument_no = this->program->music_wave_instrument;
		if (this->channel_no != 2){
			assert(this->channel_no == 6);
			instrument_no = this->program->sfx_wave_instrument;
		}
		static_assert(array_length(instruments_bank_1) == array_length(instruments_bank_2) && array_length(instruments_bank_2) == array_length(instruments_bank_3), "");
		assert(instrument_no < array_length(instruments_bank_1));
		auto &instrument = instruments_by_bank[this->bank - 1][instrument_no];
		this->program->renderer->set_NR30(0);
		this->program->renderer->copy_voluntary_wave(instrument);
		this->program->renderer->set_NR30(0x80);
	}
	assert(frequency < 0x10000 && frequency >= 0);
	if (this->program->is_cry()){
		frequency += this->program->frequency_modifier;
		frequency &= 0xFFFF;
	}
	byte_t hi = (byte_t)(frequency >> 8);
	byte_t lo = (byte_t)(frequency & 0xFF);
	this->program->set_register(RegisterId::FrequencyLow, this->channel_no, lo);
	this->program->set_register(RegisterId::FrequencyHigh, this->channel_no, (hi | BITMAP(10000000)) & BITMAP(11000111));
}

bool CppRedAudioProgram::is_cry(){
	return this->channels[4] && this->channels[4]->is_cry();
}

bool CppRedAudioProgram::Channel::is_cry(){
	return this->program->resources[(int)this->sound_id].type == AudioResourceType::Cry;
}

void CppRedAudioProgram::play_sound(AudioResourceId id){
	if (id == AudioResourceId::None)
		return;
	this->sound_id = id;
	if (id == AudioResourceId::Stop){
		//Turn on sound hardware.
		this->renderer->set_NR52(0x80);
		//Turn on voluntary wave.
		this->renderer->set_NR30(0x80);
		//Mute using stereo panning register.
		this->renderer->set_NR51(0);
		//Mute voluntary wave.
		this->renderer->set_NR32(0);
		//Disable frequency sweep of square wave 1.
		this->renderer->set_NR10(8);
		//Mute square wave 1.
		this->renderer->set_NR12(8);
		//Mute square wave 2.
		this->renderer->set_NR22(8);
		//Mute noise.
		this->renderer->set_NR42(8);
		this->renderer->set_NR14(0x40);
		this->renderer->set_NR24(0x40);
		this->renderer->set_NR44(0x40);
		//Set full volume.
		this->renderer->set_NR50(0x77);

		this->disable_channel_output_when_sfx_ends = false;
		this->pause_music_state = PauseMusicState::NotPaused;
		
		this->music_wave_instrument = 0;
		this->sfx_wave_instrument = 0;
		for (auto &c : this->channels)
			c.reset();
		this->music_tempo = 0x100;
		this->sfx_tempo = 0x100;
		this->stereo_panning = 0xFF;
		return;
	}
	auto offset = (size_t)id;
	assert(!!offset);
	if (offset >= this->resources.size()){
		std::stringstream stream;
		stream << "Invalid AudioResourceId: " << offset + 1;
		throw std::runtime_error(stream.str());
	}
	auto &resource = this->resources[offset];
	this->current_resource = &this->resources[offset];
	if (resource.type == AudioResourceType::Music){
		//play music
		this->disable_channel_output_when_sfx_ends = false;
		this->music_tempo -= this->music_tempo & 0xFF;
		this->music_wave_instrument = 0;
		this->sfx_wave_instrument = 0;
		for (auto &c : this->channels)
			c.reset();
		this->music_tempo = 1;
		this->stereo_panning = 0xFF;
		this->renderer->set_NR50(0);
		this->renderer->set_NR10(8);
		this->renderer->set_NR51(0);
		this->renderer->set_NR30(0);
		this->renderer->set_NR30(0x80);
		this->renderer->set_NR50(0x77);

		for (size_t i = 0; i < this->current_resource->channel_count; i++){
			auto &channel = this->current_resource->channels[i];
			auto &c = this->channels[channel.channel];
			c.reset(new Channel(*this, channel.channel, id, channel.entry_point, this->current_resource->bank));
		}
	}else{
		//play SFX
		for (size_t i = 0; i < this->current_resource->channel_count; i++){
			auto &channel = this->current_resource->channels[i];
			auto &c = this->channels[channel.channel];
			if (c){
				bool skip_check = false;
				if (channel.channel == 7){
					if (this->current_resource->type == AudioResourceType::NoiseInstrument)
						return;
					if (this->resources[(int)c->get_sound_id()].type == AudioResourceType::NoiseInstrument)
						skip_check = true;
				}
				if (!skip_check && id > c->get_sound_id())
					return;
				c->reset(id, channel.entry_point, this->current_resource->bank);
			}else
				c.reset(new Channel(*this, channel.channel, id, channel.entry_point, this->current_resource->bank));
			if (channel.channel == 4)
				this->renderer->set_NR10(8);
		}
	}
	if (this->current_resource->type == AudioResourceType::Cry && this->channels[6]){
		//Overwrite the program counter of channel 6 to make it terminate immediately on the next
		//update, or just destroy the object if that's somehow not possible.
		for (int pc = 0; ; pc++){
			assert(this->commands.size() <= std::numeric_limits<int>::max());
			if (pc == this->commands.size()){
				//This should never happen.
				this->channels[6].reset();
				break;
			}
			if (this->commands[pc].type == AudioCommandType::End){
				this->channels[6]->set_program_counter(pc);
				break;
			}
		}
	}
	if (!this->saved_volume){
		this->saved_volume = this->renderer->get_NR50();
		//Set full volume.
		this->renderer->set_NR50(0x77);
	}
}

CppRedAudioProgram::Channel::Channel(CppRedAudioProgram &program, int channel_no, AudioResourceId resource_id, int entry_point, int bank){
	this->program = &program;
	this->channel_no = channel_no;
	this->reset(resource_id, entry_point, bank);
}

void CppRedAudioProgram::Channel::reset(AudioResourceId resource_id, int entry_point, int bank){
	this->program_counter = entry_point;
	this->bank = bank;
	this->sound_id = resource_id;
	this->call_stack.clear();

	//flags1
	this->perfect_pitch = false;
	this->do_noise_or_sfx = this->channel_no >= 3;
	this->vibrato_direction = false;
	this->do_pitch_bend = false;
	this->pitch_bend_decreasing = false;
	this->do_rotate_duty = false;

	this->duty = 0;
	this->duty_cycle = 0;
	this->vibrato_delay_counter = 0;
	this->vibrato_extent = 0;
	this->vibrato_length = this->vibrato_counter = 0;
	this->channel_frequency = 0;
	this->vibrato_delay_counter_reload_value = 0;
	this->pitch_bend_length = 0;
	this->pitch_bend_advance = 0;
	this->pitch_bend_current_frequency = 0;
	this->pitch_bend_target_frequency = 0;

	//flags2
	this->do_execute_music = false;

	this->loop_counter = 1;
	this->note_delay_counter = 1;
	this->note_speed = 1;

	//Other things:
	this->note_delay_counter_fractional_part = 0;
	this->vibrato_counter = 0;
	this->volume = 0;
	this->fade = 0;
	this->octave = 5;
	this->do_rotate_duty = false;
	this->ifred_execute_bit = true;
}

#define DEFINE_REGISTER_FUNCTION(reg, ch, dst) \
static byte_t register_function_##reg##_##ch(AudioRenderer &renderer, int i){ \
	if (i < 0) \
		return renderer.get_NR##dst(); \
	renderer.set_NR##dst(i); \
	return 0; \
}

DEFINE_REGISTER_FUNCTION(DutySoundLength, 0, 11)
DEFINE_REGISTER_FUNCTION(DutySoundLength, 1, 21)
DEFINE_REGISTER_FUNCTION(DutySoundLength, 2, 31)
DEFINE_REGISTER_FUNCTION(DutySoundLength, 3, 41)

DEFINE_REGISTER_FUNCTION(VolumeEnvelope, 0, 12)
DEFINE_REGISTER_FUNCTION(VolumeEnvelope, 1, 22)
DEFINE_REGISTER_FUNCTION(VolumeEnvelope, 2, 32)
DEFINE_REGISTER_FUNCTION(VolumeEnvelope, 3, 42)

DEFINE_REGISTER_FUNCTION(FrequencyLow, 0, 13)
DEFINE_REGISTER_FUNCTION(FrequencyLow, 1, 23)
DEFINE_REGISTER_FUNCTION(FrequencyLow, 2, 33)
DEFINE_REGISTER_FUNCTION(FrequencyLow, 3, 43)

DEFINE_REGISTER_FUNCTION(FrequencyHigh, 0, 14)
DEFINE_REGISTER_FUNCTION(FrequencyHigh, 1, 24)
DEFINE_REGISTER_FUNCTION(FrequencyHigh, 2, 34)
DEFINE_REGISTER_FUNCTION(FrequencyHigh, 3, 44)

CppRedAudioProgram::register_function CppRedAudioProgram::get_register_pointer(RegisterId id, int channel_no){
	assert(channel_no >= 0 && channel_no < 8);
	channel_no %= 4;
	auto index = channel_no + ((int)id - 1) * 4;
	static const register_function functions[] = {
		register_function_DutySoundLength_0,
		register_function_DutySoundLength_1,
		register_function_DutySoundLength_2,
		register_function_DutySoundLength_3,
		register_function_VolumeEnvelope_0,
		register_function_VolumeEnvelope_1,
		register_function_VolumeEnvelope_2,
		register_function_VolumeEnvelope_3,
		register_function_FrequencyLow_0,
		register_function_FrequencyLow_1,
		register_function_FrequencyLow_2,
		register_function_FrequencyLow_3,
		register_function_FrequencyHigh_0,
		register_function_FrequencyHigh_1,
		register_function_FrequencyHigh_2,
		register_function_FrequencyHigh_3,
	};
	if (index < 0 || index >= array_length(functions))
		throw std::exception();
	return functions[index];
}

void CppRedAudioProgram::clear_channel(int channel){
	this->channels[euclidean_modulo(channel, (int)array_length(this->channels))].reset();
}

std::vector<std::string> CppRedAudioProgram::get_resource_strings(){
	std::vector<std::string> ret;
	ret.reserve(this->resources.size());
	for (auto &r : this->resources)
		ret.push_back(r.name);
	return ret;
}

void CppRedAudioProgram::copy_fade_control(){
	this->fade_out_counter = this->fade_out_counter_reload_value = this->fade_out_control;
}

std::unique_lock<std::mutex> CppRedAudioProgram::acquire_lock(){
	return std::unique_lock<std::mutex>(this->mutex);
}

bool CppRedAudioProgram::is_sfx_playing(){
	bool any = false;
	for (int i = 4; i < array_length(this->channels) && !any; i++)
		any = !!this->channels[i];
	return any;
}

void CppRedAudioProgram::wait_for_sfx_to_end(){
	{
		LOCK_MUTEX(this->mutex);
		if (!this->is_sfx_playing())
			return;
	}
	this->sfx_finish_event.reset_and_wait();
}
