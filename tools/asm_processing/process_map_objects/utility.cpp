#include "utility.h"
#include <fstream>

std::vector<char> load_file(const std::string &path){
	std::ifstream file(path, std::ios::binary);
	file.seekg(0, std::ios::end);
	std::vector<char> ret(file.tellg());
	file.seekg(0);
	file.read(&ret[0], ret.size());
	return ret;
}

std::deque<char> normalize_file_contents(const std::vector<char> &buffer){
	std::deque<char> ret;
	for (auto c : buffer){
		switch (c){
			case '\t':
			case '\v':
			case '\f':
				c = ' ';
				break;
			case '\r':
				c = '\n';
				break;
		}
		ret.push_back(c);
	}
	return ret;
}
