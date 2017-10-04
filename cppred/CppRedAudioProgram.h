#pragma once
#include "Audio.h"
#include "CppRedData.h"

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

struct AudioCommand{
	AudioCommandType type;
	std::uint32_t params[4];
};

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
DEFINE_AC_STRUCT1(DSpeed, dspeed);
DEFINE_AC_STRUCT2(Snare, type, length);
DEFINE_AC_STRUCT2(MutedSnare, type, length);
DEFINE_AC_STRUCT1(UnknownSfx10, param);
DEFINE_AC_STRUCT4(UnknownSfx20, param1, param2, param3, param4);
DEFINE_AC_STRUCT3(UnknownNoise20, param1, param2, param3);
DEFINE_AC_STRUCT0(ExecuteMusic);
DEFINE_AC_STRUCT2(PitchBend, length, frequency);
DEFINE_AC_STRUCT2(Triangle, type, length);
DEFINE_AC_STRUCT1(StereoPanning, value);
DEFINE_AC_STRUCT2(Cymbal, type, length);
DEFINE_AC_STRUCT2(Loop, times, dst);
DEFINE_AC_STRUCT1(Call, dst);
DEFINE_AC_STRUCT1(Goto, dst);
DEFINE_AC_STRUCT0(IfRed);
DEFINE_AC_STRUCT0(Else);
DEFINE_AC_STRUCT0(EndIf);
DEFINE_AC_STRUCT0(End);

class CppRedAudioProgram : public AudioProgram{
	std::vector<AudioCommand> commands;
	int channel_sound_ids[8];
	byte_t mute_audio_and_pause_music = 0;
	byte_t channel_note_delay_counters[8];

	void apply_effects(int channel, double now, AbstractAudioRenderer &renderer);
	void load_commands();
public:
	CppRedAudioProgram();
	void update(double now, AbstractAudioRenderer &renderer);
};
