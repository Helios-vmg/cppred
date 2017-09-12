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

class PartyData{
	byte_t *memory;
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type u8_type;
	typedef u8_type::callback_struct callback_struct;
	static const size_t size = 404;

	PartyData(byte_t *memory);
#include "../CodeGeneration/output/party_data.h"
	void copy_from(const PartyData &src);
};

class MainData{
	byte_t *memory;
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type u8_type;
	typedef u8_type::callback_struct callback_struct;
	static const size_t size = 1929;

	MainData(byte_t *memory);
#include "../CodeGeneration/output/main_data.h"
	void copy_from(const MainData &src);
};

class BoxData{
	byte_t *memory;
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type u8_type;
	typedef u8_type::callback_struct callback_struct;
	static const size_t size = 1122;

	BoxData(byte_t *memory);
#include "../CodeGeneration/output/box_data.h"
	void copy_from(const BoxData &src);
};

class SpriteData{
	byte_t *memory;
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type u8_type;
	typedef u8_type::callback_struct callback_struct;
	static const size_t size = 512;

	SpriteData(byte_t *memory);
#include "../CodeGeneration/output/sprite_data.h"
	void copy_from(const SpriteData &src);
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
