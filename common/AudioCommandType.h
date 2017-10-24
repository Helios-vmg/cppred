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
	NoiseInstrument,
	UnknownSfx10,
	UnknownSfx20,
	UnknownNoise20,
	ExecuteMusic,
	PitchBend,
	StereoPanning,
	Loop,
	Call,
	Goto,
	IfRed,
	Else,
	EndIf,
	End,
};
