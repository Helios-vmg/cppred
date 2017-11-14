#pragma once
#include <vector>
#include <string>
#include <cstdint>

typedef std::uint8_t byte_t;

std::vector<byte_t> base64_decode(const std::string &in);
std::string base64_encode(const std::vector<byte_t> &in);
