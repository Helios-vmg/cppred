#pragma once
#include <string>
#include <vector>
#include <queue>

std::vector<char> load_file(const std::string &path);
std::deque<char> normalize_file_contents(const std::vector<char> &buffer);
