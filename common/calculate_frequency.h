#pragma once
#include <cstdint>

inline std::uint32_t calculate_frequency(std::uint32_t note, std::uint32_t octave){
	static const std::uint32_t pitches[] = {
		63532,
		63645,
		63751,
		63851,
		63946,
		64035,
		64119,
		64199,
		64274,
		64344,
		64411,
		64474,
	};
	auto pitch = pitches[note];
	while (octave++ < 7)
		pitch = ((pitch << 15) & 0x8000) | ((pitch >> 1) & 0x7FFF);
	return (pitch + 0x0800) & 0xFFFF;
}
