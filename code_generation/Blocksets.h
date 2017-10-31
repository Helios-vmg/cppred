#pragma once
#include <map>
#include <vector>
#include <cstdint>
#include <memory>
#include "utility.h"

typedef std::uint8_t byte_t;

class Blocksets{
	std::map<std::string, std::shared_ptr<std::vector<byte_t>>> blocksets;
public:
	Blocksets(const char *path);
	DELETE_COPY_CONSTRUCTORS(Blocksets);
	std::shared_ptr<std::vector<byte_t>> get(const std::string &) const;
};
