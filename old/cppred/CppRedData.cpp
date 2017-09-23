#pragma once
#include "CppRedData.h"
#include "utility.h"
#include "CppRedText.h"

#define AT_LEVEL(level, next_form) EvolutionTriggerType::Level, level, SpeciesId::next_form
#define WITH_ITEM(item, level, next_form) EvolutionTriggerType::Item, level, SpeciesId::next_form, ItemId::item
#define WHEN_TRADED(level, next_form) EvolutionTriggerType::Trade, level, SpeciesId::next_form
#define LEARN(level, move) level, MoveId::move

#include "../CodeGeneration/output/pokemon_definitions.inl"
#include "../CodeGeneration/output/move_data.inl"

//Note: Palettes 3 and 4 are identical and equal to the default palette. Palettes >= 4 are used for fade-outs
//      to white, while palettes <= 3 are used for fade-outs to black.
const FadePaletteData fade_palettes[8] = {
	{ bits_from_u32<0x11111111>::value, bits_from_u32<0x11111111>::value, bits_from_u32<0x11111111>::value },
	{ bits_from_u32<0x11111110>::value, bits_from_u32<0x11111110>::value, bits_from_u32<0x11111000>::value },
	{ bits_from_u32<0x11111001>::value, bits_from_u32<0x11100100>::value, bits_from_u32<0x11100100>::value },
	{ bits_from_u32<0x11100100>::value, bits_from_u32<0x11010000>::value, bits_from_u32<0x11100000>::value },
	{ bits_from_u32<0x11100100>::value, bits_from_u32<0x11010000>::value, bits_from_u32<0x11100000>::value },
	{ bits_from_u32<0x10010000>::value, bits_from_u32<0x10000000>::value, bits_from_u32<0x10010000>::value },
	{ bits_from_u32<0x01000000>::value, bits_from_u32<0x01000000>::value, bits_from_u32<0x01000000>::value },
	{ bits_from_u32<0x00000000>::value, bits_from_u32<0x00000000>::value, bits_from_u32<0x00000000>::value },
};
