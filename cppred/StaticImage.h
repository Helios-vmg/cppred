#pragma once
#include "CommonTypes.h"
#include <vector>

enum class BitmapEncoding{
	Bit1,
	Bit2,
	Rgb,
};

template <size_t N>
struct StaticImage{
	std::uint8_t width;
	std::uint8_t height;
	BitmapEncoding encoding;
	//Encoded row major. LSB is left-most pixel.
	byte_t data[N];
	static const size_t data_size = N;
};

std::vector<byte_t> decode_image_data(unsigned w, unsigned h, BitmapEncoding encoding, const byte_t *data, size_t size);

template <size_t N>
std::vector<byte_t> decode_image_data(const StaticImage<N> &img){
	return decode_image_data(img.w, img.h, img.encoding, img.data, N);
}
