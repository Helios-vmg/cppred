#include "CppRedSRam.h"
#include "utility.h"

void SRamExtraBoxData::clear(){
	memset(this->memory, 0, size);
}

void SRam::clear(){
	memset(this->memory, 0, size);
}

void SRam::clear(xorshift128_state &state){
	auto sram = (byte_t *)this->memory;
	for (size_t i = 0; i < size;){
		auto u = xorshift128(state);
		for (int j = 4; j--; i++){
			sram[i] = u & 0xFF;
			u >>= 8;
		}
	}
}
