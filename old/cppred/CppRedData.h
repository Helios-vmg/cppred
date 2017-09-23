#pragma once

#include "CommonTypes.h"

#include "../CodeGeneration/output/pokemon_declarations.h"
#include "../CodeGeneration/output/move_data.h"

#pragma pack(push)
#pragma pack(1)
struct FadePaletteData{
	byte_t background_palette;
	byte_t obp0_palette;
	byte_t obp1_palette;
};
#pragma pack(pop)

extern const FadePaletteData fade_palettes[8];
