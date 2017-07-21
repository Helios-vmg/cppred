#pragma once

#include "CommonTypes.h"
#include <memory>

template <main_integer_t START>
class MemorySection{
	std::unique_ptr<byte_t[]> pointer;
	size_t size;
	byte_t *memory;
public:
	MemorySection(size_t size): pointer(new byte_t[size]), size(size){
		this->memory = this->pointer.get();
	}
	byte_t &access(main_integer_t address){
		return this->memory[address - START];
	}
	const byte_t &access(main_integer_t address) const{
		return this->memory[address - START];
	}
	void copy_from(const MemorySection<START> &src){
		size_t length = std::min(src.size, this->size);
		memcy(this->memory, src.memory, length);
	}
};
