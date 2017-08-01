#pragma once
#include "CppRedStructs.h"
#include "CommonTypes.h"
#include "CppRedConstants.h"
#include "CppRedMap.h"
#include <memory>

class RamRegion{
	std::unique_ptr<byte_t[]> ptr;
	size_t size;
protected:
	byte_t *memory;
public:
	RamRegion(size_t memory_size);
	virtual ~RamRegion() = 0;
	size_t get_size() const{
		return this->size;
	}
	void clear();
};

class WRam : public RamRegion{
public:
	WRam();
	~WRam(){}
#include "../CodeGeneration/output/wram.h"
};

class HRam : public RamRegion{
public:
	HRam();
	~HRam(){}
#include "../CodeGeneration/output/hram.h"
};

void array_overflow();
