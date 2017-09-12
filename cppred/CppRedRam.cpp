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

PartyData::PartyData(byte_t *memory):
	memory(memory),
#include "../CodeGeneration/output/party_data.inl"
{}

MainData::MainData(byte_t *memory) :
	memory(memory),
#include "../CodeGeneration/output/main_data.inl"
{}

BoxData::BoxData(byte_t *memory) :
	memory(memory),
#include "../CodeGeneration/output/box_data.inl"
{}

SpriteData::SpriteData(byte_t *memory) :
	memory(memory),
#include "../CodeGeneration/output/sprite_data.inl"
{}

void MainData::copy_from(const MainData &src){
	memcpy(this->memory, src.memory, size);
}

void SpriteData::copy_from(const SpriteData &src){
	memcpy(this->memory, src.memory, size);
}

void BoxData::copy_from(const BoxData &src){
	memcpy(this->memory, src.memory, size);
}

void PartyData::copy_from(const PartyData &src){
	memcpy(this->memory, src.memory, size);
}
