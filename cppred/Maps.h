#pragma once

#include "common_types.h"
#include "GraphicsAsset.h"
#include "../CodeGeneration/output/maps.h"
#include "../common/TilesetType.h"
#include <vector>

struct TilesetData{
	const char *name;
	Blocksets::pair_t blockset;
	const GraphicsAsset *tiles;
	Collision::pair_t collision;
	std::vector<int> counters;
	int grass_tile;
	TilesetType type;
};

struct MapData{
	const char *name;
	const TilesetData *tileset;
	int width, height;
	BinaryMapData::pair_t map_data;
};
