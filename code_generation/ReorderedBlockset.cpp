#include "ReorderedBlockset.h"
#include <cassert>

static int reorder(int i){
	return i + 2 * (i / 2 % 2 - i / 4 % 2);
}

ReorderedBlockset::ReorderedBlockset(const std::vector<byte_t> &blockset){
	assert(!(blockset.size() % 16));

	std::map<Block, std::vector<std::pair<int, int>>> tile_allocator;

	for (size_t i = 0; i < blockset.size(); i += 16){
		Block tiles[4];
		for (int j = 0; j < 16; j++)
			tiles[j / 4].tiles[j % 4] = blockset[i + reorder(j)];
		int j = 0;
		for (auto &t : tiles){
			auto it = tile_allocator.find(t);
			if (it == tile_allocator.end()){
				tile_allocator[t];
				it = tile_allocator.find(t);
			}
			it->second.emplace_back(i / 16, j);
			j++;
		}
	}

	for (auto &kv : tile_allocator){
		for (auto &pair : kv.second)
			this->block_renames[pair] = tiles.size();
		this->tiles.push_back(kv.first);
	}
}
