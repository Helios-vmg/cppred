#pragma once

#include "CommonTypes.h"
#include <array>

void read_memory_u8(byte_t &dst, const void *mem);
void write_memory_u8(const byte_t &src, void *mem);

void read_memory_u16(std::uint16_t &dst, const void *mem);
void write_memory_u16(const std::uint16_t &src, void *mem);

void read_memory_u24(std::uint32_t &dst, const void *mem);
void write_memory_u24(const std::uint32_t &src, void *mem);

void read_memory_u16_big(std::uint16_t &dst, const void *mem);
void write_memory_u16_big(const std::uint16_t &src, void *mem);

void read_memory_u24_big(std::uint32_t &dst, const void *mem);
void write_memory_u24_big(const std::uint32_t &src, void *mem);

void read_memory_u32_big(std::uint32_t &dst, const void *mem);
void write_memory_u32_big(const std::uint32_t &src, void *mem);

template <typename T, size_t N>
void read_memory(std::array<T, N> &dst, const void *mem, size_t element_size){
	auto m = (const byte_t *)mem;
	for (auto &i : dst){
		read_memory(i, m);
		m += element_size;
	}
}

template <typename T, size_t N>
void write_memory(const std::array<T, N> &src, void *mem, size_t element_size){
	auto m = (byte_t *)mem;
	for (auto &i : src){
		write_memory(i, m);
		m += element_size;
	}
}

void read_bcd4(unsigned &dst, const void *mem);
void write_bcd4(const unsigned &src, void *mem);

void read_bcd6(unsigned &dst, const void *mem);
void write_bcd6(const unsigned &src, void *mem);
