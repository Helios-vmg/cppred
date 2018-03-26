#pragma once
#include "common_types.h"
#ifndef HAVE_PCH
#include <cstdint>
#endif

struct GraphicsAsset{
	std::uint16_t first_tile;
	byte_t width, height;
};
