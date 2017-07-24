#include "CppRedRam.h"
#include "MemoryOperations.h"
#include <stdexcept>

void array_overflow(){
	throw std::runtime_error("Array overflow!");
}


RamRegion::RamRegion(size_t memory_size): ptr(new byte_t[memory_size]){
	this->size = memory_size;
	this->memory = this->ptr.get();
}

RamRegion::~RamRegion(){
}

void RamRegion::clear(){
	memset(this->memory, 0, this->size);
}

WRam::WRam(): RamRegion(wram_size),
#include "../CodeGeneration/output/wram.inl"
{}

HRam::HRam(): RamRegion(hram_size),
#include "../CodeGeneration/output/hram.inl"
{}
