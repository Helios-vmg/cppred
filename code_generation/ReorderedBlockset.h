#pragma once
#include <vector>
#include <map>
#include <cstdint>

typedef std::uint8_t byte_t;

struct Block{
	static const size_t size = 4;
	byte_t tiles[size];
	Block(){
		memset(this->tiles, 0, size);
	}
	Block(const Block &other){
		memcpy(this->tiles, other.tiles, size);
	}
	bool operator<(const Block &other) const{
		return memcmp(this->tiles, other.tiles, size) < 0;
	}
};

struct ReorderedBlockset{
	std::vector<Block> tiles;
	std::map<std::pair<int, int>, int> block_renames;
	ReorderedBlockset() = default;
	ReorderedBlockset(const std::vector<byte_t> &blockset);
};
