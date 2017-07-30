#pragma once

#include "CommonTypes.h"

struct PokemonCryData{
	byte_t base_cry;
	byte_t pitch;
	byte_t length;
};

extern const PokemonCryData pokemon_cry_data[];
extern const size_t pokemon_cry_data_size;

#pragma pack(push)
#pragma pack(1)
struct FadePaletteData{
	byte_t background_palette;
	byte_t obp0_palette;
	byte_t obp1_palette;
};
#pragma pack(pop)

extern const FadePaletteData fade_palettes[8];
