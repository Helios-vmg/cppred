#include "MemoryOperations.h"

template <int N, typename T>
void read_bytes(T &dst, const void *mem){
	auto p = (const byte_t *)mem + N;
	dst = 0;
	for (int i = N; i--;){
		dst <<= 8;
		dst |= *--p;
	}
}

template <int N, typename T>
void write_bytes(const T &src, void *mem){
	auto p = (byte_t *)mem;
	auto copy = src;
	for (int i = N; i--;){
		*p++ = copy & 0xFF;
		copy >>= 8;
	}
}

template <int N, typename T>
void read_bytes_big(T &dst, const void *mem){
	auto p = (const byte_t *)mem;
	dst = 0;
	for (int i = N; i--;){
		dst <<= 8;
		dst |= *p++;
	}
}

template <int N, typename T>
void write_bytes_big(const T &src, void *mem){
	auto p = (byte_t *)mem + N;
	auto copy = src;
	for (int i = N; i--;){
		*--p = copy & 0xFF;
		copy >>= 8;
	}
}

template <int N, typename T>
void read_bcd(T &dst, const void *mem){
	T temp;
	read_bytes_big<N>(temp, mem);
	dst = 0;
	T multiplier = 1;
	for (int i = N * 2; i--;){
		dst += (temp & 0x0F) * multiplier;
		multiplier *= 10;
		temp >>= 16;
	}
}

template <int N, typename T>
void write_bcd(const T &src, void *mem){
	T temp = 0;
	T copy = src;
	int shift = 0;
	for (int i = N * 2; i--;){
		temp |= (copy % 10) << shift;
		shift += 4;
		copy /= 10;
	}
	read_bytes_big<N>(temp, mem);
}

void read_memory_u8(byte_t &dst, const void *mem){
	read_bytes<1>(dst, mem);
}

void write_memory_u8(const byte_t &src, void *mem){
	write_bytes<1>(src, mem);
}

void read_memory_u16(std::uint16_t &dst, const void *mem){
	read_bytes<2>(dst, mem);
}

void write_memory_u16(const std::uint16_t &src, void *mem){
	write_bytes<2>(src, mem);
}

void read_memory_u24(std::uint32_t &dst, const void *mem){
	read_bytes<3>(dst, mem);
}

void write_memory_u24(const std::uint32_t &src, void *mem){
	write_bytes<3>(src, mem);
}

void read_memory_u16_big(std::uint16_t &dst, const void *mem){
	read_bytes_big<2>(dst, mem);
}

void write_memory_u16_big(const std::uint16_t &src, void *mem){
	write_bytes_big<2>(src, mem);
}

void read_memory_u24_big(std::uint32_t &dst, const void *mem){
	read_bytes_big<3>(dst, mem);
}

void write_memory_u24_big(const std::uint32_t &src, void *mem){
	write_bytes_big<3>(src, mem);
}

void read_memory_u32_big(std::uint32_t &dst, const void *mem){
	read_bytes_big<4>(dst, mem);
}

void write_memory_u32_big(const std::uint32_t &src, void *mem){
	write_bytes_big<4>(src, mem);
}

void read_bcd4(unsigned &dst, const void *mem){
	read_bcd<2>(dst, mem);
}

void write_bcd4(const unsigned &src, void *mem){
	write_bcd<2>(src, mem);
}

void read_bcd6(unsigned &dst, const void *mem){
	read_bcd<3>(dst, mem);
}

void write_bcd6(const unsigned &src, void *mem){
	write_bcd<3>(src, mem);
}
