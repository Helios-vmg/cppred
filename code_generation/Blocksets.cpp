#include "Blocksets.h"
#include "utility.h"
#include <stdexcept>

Blocksets::Blocksets(const char *path){
	this->blocksets = read_data_csv(path);
}

std::shared_ptr<std::vector<byte_t>> Blocksets::get(const std::string &name) const{
	auto it = this->blocksets.find(name);
	if (it == this->blocksets.end())
		throw std::runtime_error("Error: Invalid blockset name \"" + name + "\"");
	return it->second;
}
