#include "StaticImage.h"
#include "utility.h"
#include "CppRedConstants.h"
#include <cassert>

std::vector<byte_t> decode_image_data(unsigned w, unsigned h, BitmapEncoding encoding, const byte_t *data, size_t size, bool flip){
	std::vector<byte_t> ret;
	if (encoding == BitmapEncoding::Bit1){
		ret.resize(w * h / 8);
		for (unsigned y = 0; y < h; y++){
			for (unsigned x = 0; x < w; x++){
				unsigned src_x = !flip ? x : (w - 1) - x;
				bool src = check_flag<unsigned>(data[(y * w + src_x) / 8], src_x % 8);
				if (!(x % 8))
					ret.resize(ret.size() + 2, 0);
				if (src){
					ret[ret.size() - 2] |= 1 << (x % 8);
					ret[ret.size() - 1] |= 1 << (x % 8);
				}
			}
		}
	}else if (encoding == BitmapEncoding::Bit2){
		for (unsigned y = 0; y < h; y++){
			for (unsigned x = 0; x < w; x++){
				unsigned src_x = !flip ? x : (w - 1) - x;
				bool src0 = check_flag<unsigned>(data[(y * w + src_x) / 4], (src_x * 2 + 0) % 8);
				bool src1 = check_flag<unsigned>(data[(y * w + src_x) / 4], (src_x * 2 + 1) % 8);
				if (!(x % 8))
					ret.resize(ret.size() + 2, 0);
				ret[ret.size() - 2] |= (byte_t)src0 << (x % 8);
				ret[ret.size() - 1] |= (byte_t)src1 << (x % 8);
			}
		}
	}
	return ret;
}

std::vector<byte_t> decode_image_data(const BaseStaticImage &img, bool flip){
	return decode_image_data(img.get_width() * tile_pixel_size, img.get_height() * tile_pixel_size, img.get_encoding(), img.get_data(), img.get_data_size(), flip);
}

std::vector<byte_t> pad_tiles_for_mon_pic(const std::vector<byte_t> &image_data, unsigned tiles_w, unsigned tiles_h, bool flipped){
	assert(tiles_w <= 7 && tiles_h <= 7);
	std::vector<byte_t> ret(7 * 7 * 16);
	unsigned y0 = 7 - tiles_h;
	unsigned x0 = ((!flipped ? 8 : 7) - tiles_w) / 2;
	for (unsigned dst_y = y0; dst_y < 7; dst_y++){
		unsigned src_y = dst_y - y0;
		assert(src_y < tiles_h);
		for (unsigned x = x0; x < tiles_w; x++){
			auto src = &image_data[(x + src_y * tiles_w) * 16];
			auto dst = &ret[(x + dst_y * tiles_h) * 16];
			memcpy(dst, src, 16);
		}
	}
	return ret;
}
