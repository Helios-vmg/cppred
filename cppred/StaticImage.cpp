#include "StaticImage.h"
#include "utility.h"

std::vector<byte_t> decode_image_data(unsigned w, unsigned h, BitmapEncoding encoding, const byte_t *data, size_t size){
	std::vector<byte_t> ret;
	if (encoding == BitmapEncoding::Bit1){
		ret.resize(w * h / 8);
		for (unsigned y = 0; y < h; y++){
			for (unsigned x = 0; x < w; x++){
				bool src = check_flag<unsigned>(data[(y * w + x) / 8], x % 8);
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
				bool src0 = check_flag<unsigned>(data[(y * w + x) / 4], (x * 2 + 0) % 8);
				bool src1 = check_flag<unsigned>(data[(y * w + x) / 4], (x * 2 + 1) % 8);
				if (!(x % 8))
					ret.resize(ret.size() + 2, 0);
				ret[ret.size() - 2] |= (byte_t)src0 << (x % 8);
				ret[ret.size() - 1] |= (byte_t)src1 << (x % 8);
			}
		}
	}
	return ret;
}

std::vector<byte_t> decode_image_data(const BaseStaticImage &img){
	return decode_image_data(img.get_width(), img.get_height(), img.get_encoding(), img.get_data(), img.get_data_size());
}
