#pragma once
#include <cstdint>

typedef std::uint8_t byte_t;

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
