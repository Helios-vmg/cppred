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

//'flip' mirrors the pixels horizontally.
std::vector<byte_t> decode_image_data(const BaseStaticImage &img, bool flip = false);
//Note: 'flipped' does not mirror the pixels. Set it to true if the image data
//      was decoded by passing flip = true to decode_image_data().
std::vector<byte_t> pad_tiles_for_mon_pic(const std::vector<byte_t> &image_data, unsigned tiles_w, unsigned tiles_h, bool flip = false);

template <typename T>
void write_mon_pic_tiles_to_buffer(const T &dst, unsigned pitch){
	unsigned value = 0;
	for (unsigned y = 0; y < 7; y++)
		for (unsigned x = 0; x < 7; x++)
			dst[x + y * pitch] = value++;
}
