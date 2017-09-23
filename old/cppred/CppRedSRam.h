#pragma once
#include "CppRedRam.h"
#include "utility.h"

class SRam;

class SRamExtraBoxData{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type u8_type;
	typedef u8_type::callback_struct callback_struct;
	static const size_t size = 0x2000;
private:
	void *memory;
public:
	//Offset: 0
	WrappedArray<BoxData, 6, BoxData::size> boxes;
	//Offset: 6732 (0x1a4c)
	u8_type boxes_checksum;
	//Offset: 6733 (0x1a4d)
	WrappedArray<typename u8_type::type, 6, u8_type::size> individual_checksums;
	//Offset: 6739 (0x1a53)
	WrappedArray<typename u8_type::type, 1453, u8_type::size> padding;

	SRamExtraBoxData(void *memory, const callback_struct &callbacks):
		memory(memory),
		boxes               ((byte_t *)memory + 0x0000, callbacks),
		boxes_checksum      ((byte_t *)memory + 0x1A4C, callbacks),
		individual_checksums((byte_t *)memory + 0x1A4D, callbacks),
		padding             ((byte_t *)memory + 0x1A53, callbacks)
	{}
	SRamExtraBoxData(const SRamExtraBoxData &) = default;
	SRamExtraBoxData(SRamExtraBoxData &&other): SRamExtraBoxData((const SRamExtraBoxData &)other){}
	void operator=(const SRamExtraBoxData &) = delete;
	void operator=(SRamExtraBoxData &&) = delete;
	void clear();
};

class SRam{
public:
	typedef typename WrapperSelector<std::uint8_t, 1>::type u8_type;
	//typedef typename WrapperSelector<std::uint16_t, 2>::type u16_type;
	//typedef typename WrapperSelector<std::uint32_t, 3>::type u24_type;
	struct callback_struct{
		u8_type::callback_struct cb8;
		//u16_type::callback_struct cb16;
		//u24_type::callback_struct cb24;
	};
	static const size_t size = 0x8000;
private:
	void *memory;
public:
	//Bank 0
	//Offset: 0
	WrappedArray<typename u8_type::type, 7 * 7 * 8, u8_type::size> sprite_buffer0;
	//Offset: 392 (0x188)
	WrappedArray<typename u8_type::type, 7 * 7 * 8, u8_type::size> sprite_buffer1;
	//Offset: 784 (0x310)
	WrappedArray<typename u8_type::type, 7 * 7 * 8, u8_type::size> sprite_buffer2;
	//Offset: 1176 (0x498)
	WrappedArray<typename u8_type::type, 0x100, u8_type::size> padding1;
	//Offset: 1432 (0x598)
	WrappedArray<typename u8_type::type, hall_of_fame_team * hall_of_fame_capacity, u8_type::size> hall_of_fame;
	//Offset: 6232 (0x1858)
	WrappedArray<typename u8_type::type, 1960, u8_type::size> padding2;

	//Bank 1
	//Offset: 8192 (0x2000)
	WrappedArray<typename u8_type::type, 0x598, u8_type::size> padding3;
	//Offset: 9624 (0x2598)
	WrappedArray<typename u8_type::type, character_name_length, u8_type::size> player_name;
	//Offset: 9635 (0x25a3)
	MainData main_data;
	//Offset: 11564 (0x2d2c)
	SpriteData sprite_data;
	//Offset: 12076 (0x2f2c)
	PartyData party_data;
	//Offset: 12480 (0x30c0)
	BoxData current_box_data;
	//Offset: 13602 (0x3522)
	u8_type tileset_type;
	//Offset: 13603 (0x3523)
	u8_type main_data_checksum;
	//Offset: 13604 (0x3524)
	WrappedArray<typename u8_type::type, 2780, u8_type::size> padding4;
	
	//Bank 2
	//Offset: 16384 (0x4000)
	SRamExtraBoxData boxes1;

	//Bank 3
	//Offset: 24576 (0x6000)
	SRamExtraBoxData boxes2;

	SRam(void *memory, const callback_struct &callbacks):
		memory(memory),
		sprite_buffer0    ((byte_t *)memory + 0x0000, callbacks.cb8),
		sprite_buffer1    ((byte_t *)memory + 0x0188, callbacks.cb8),
		sprite_buffer2    ((byte_t *)memory + 0x0310, callbacks.cb8),
		padding1          ((byte_t *)memory + 0x0498, callbacks.cb8),
		hall_of_fame      ((byte_t *)memory + 0x0598, callbacks.cb8),
		padding2          ((byte_t *)memory + 0x1858, callbacks.cb8),
		padding3          ((byte_t *)memory + 0x2000, callbacks.cb8),
		player_name       ((byte_t *)memory + 0x2598, callbacks.cb8),
		main_data         ((byte_t *)memory + 0x25A3),
		sprite_data       ((byte_t *)memory + 0x2D2C),
		party_data        ((byte_t *)memory + 0x2F2C),
		current_box_data  ((byte_t *)memory + 0x30C0),
		tileset_type      ((byte_t *)memory + 0x3522, callbacks.cb8),
		main_data_checksum((byte_t *)memory + 0x3523, callbacks.cb8),
		padding4          ((byte_t *)memory + 0x3524, callbacks.cb8),
		boxes1            ((byte_t *)memory + 0x4000, callbacks.cb8),
		boxes2            ((byte_t *)memory + 0x6000, callbacks.cb8)
	{}
	SRam(const SRam &) = default;
	SRam(SRam &&other): SRam((const SRam &)other){}
	void operator=(const SRam &) = delete;
	void operator=(SRam &&) = delete;
	void clear();
	void clear(xorshift128_state &);
};
