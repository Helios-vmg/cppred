#pragma once

#include "CommonTypes.h"
#include <memory>
#include <stdexcept>
#include <sstream>
#include <iomanip>

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
		if (address < START){
			std::stringstream stream;
			stream << "Attempt to access memory with base address 0x" << std::hex << std::setw('4') << std::setfill('0') << START
				<< " using address 0x" << std::hex << std::setw('4') << std::setfill('0') << ".";
			throw std::runtime_error(stream.str());
		}
		return this->memory[address - START];
	}
	const byte_t &access(main_integer_t address) const{
		if (address < START){
			std::stringstream stream;
			stream << "Attempt to access memory with base address 0x" << std::hex << std::setw('4') << std::setfill('0') << START
				<< " using address 0x" << std::hex << std::setw('4') << std::setfill('0') << ".";
			throw std::runtime_error(stream.str());
		}
		return this->memory[address - START];
	}
	void copy_from(const MemorySection<START> &src){
		size_t length = std::min(src.size, this->size);
		memcy(this->memory, src.memory, length);
	}
	void fill(byte_t value){
		memset(this->memory, value, this->size);
	}
	void clear(){
		this->fill(0);
	}
};
