#pragma once
#include "CommonTypes.h"
#include <vector>
#include <array>

enum class BitmapEncoding{
	Bit1,
	Bit2,
	Rgb,
};

class BaseStaticImage{
protected:
	std::uint8_t width;
	std::uint8_t height;
	BitmapEncoding encoding;
	const byte_t *data;
	size_t size;
public:
	BaseStaticImage(std::uint8_t w, std::uint8_t h, BitmapEncoding e, size_t s):
		width(w),
		height(h),
		encoding(e),
		size(s){}
	BaseStaticImage(const BaseStaticImage &) = delete;
	BaseStaticImage(BaseStaticImage &&) = delete;
	void operator=(const BaseStaticImage &) = delete;
	void operator=(BaseStaticImage &&) = delete;
	
	unsigned get_width() const{
		return this->width;
	}
	unsigned get_height() const{
		return this->height;
	}
	BitmapEncoding get_encoding() const{
		return this->encoding;
	}
	const byte_t *get_data() const{
		return this->data;
	}
	size_t get_data_size() const{
		return this->size;
	}
};

template <size_t N>
class StaticImage : public BaseStaticImage{
	//Encoded row major. LSB of every byte is left-most pixel. For 2-bit
	//bitmaps, the value of the left-most is stored in the least
	//significant half nibble.
	const std::array<byte_t, N> static_data;
public:
	static const size_t static_size = N;

	StaticImage(std::uint8_t w, std::uint8_t h, BitmapEncoding e, std::array<byte_t, N> &&d): BaseStaticImage(w, h, e, N), static_data(std::move(d)){
		this->data = this->static_data.data();
	}
	StaticImage(const StaticImage &) = delete;
	StaticImage(StaticImage &&) = delete;
	void operator=(const StaticImage &) = delete;
	void operator=(StaticImage &&) = delete;
};

std::vector<byte_t> decode_image_data(const BaseStaticImage &img);
