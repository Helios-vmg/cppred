#include "Image.h"
#include "../common/sha1.h"
#include "FreeImageInitializer.h"
#include <cassert>

constexpr pixel colorn(byte_t n){
	return{ n, n, n, 0xFF };
}

const pixel color3 = colorn(0x00);
const pixel color2 = colorn(0x55);
const pixel color1 = colorn(0xAA);
const pixel color0 = colorn(0xFF);

pixel::operator std::uint32_t() const{
	std::uint32_t ret;
	ret = this->r;
	ret <<= 8;
	ret |= this->g;
	ret <<= 8;
	ret |= this->b;
	ret <<= 8;
	ret |= this->a;
	return ret;
}

std::string Tile::hash() const{
	SHA1 sha1;
	sha1.Input(this->pixels, sizeof(this->pixels));
	return sha1.ToString();
}

std::set<pixel> Tile::get_unique_colors() const{
	return ::get_unique_colors(this->pixels);
}

void Image::internal_unload_bitmap(FIBITMAP *bitmap){
	if (bitmap){
		FreeImageInitializer fii;
		FreeImage_Unload(bitmap);
	}
}

Image::bitmap_ptr Image::to_ptr(FIBITMAP *bitmap){
	return bitmap_ptr(bitmap, internal_unload_bitmap);
}

Image::bitmap_ptr Image::load_bitmap(const char *path){
	FreeImageInitializer fii;
	auto type = FreeImage_GetFileType(path);
	if (type == FIF_UNKNOWN)
		return bitmap_ptr(nullptr, internal_unload_bitmap);
	return to_ptr(FreeImage_Load(type, path, type == FIF_JPEG ? JPEG_ACCURATE : 0));
}

Image::bitmap_ptr Image::convert_to_32bit_bitmap(FIBITMAP *bitmap){
	FreeImageInitializer fii;
	auto converted = to_ptr(FreeImage_ConvertTo32Bits(bitmap));
	if (converted)
		return converted;
	converted = to_ptr(FreeImage_ConvertToType(bitmap, FIT_BITMAP, true));
	if (!converted)
		return bitmap_ptr(nullptr, internal_unload_bitmap);
	return convert_to_32bit_bitmap(converted.get());
}

std::unique_ptr<Image> Image::load_image(const char *path){
	auto bitmap = load_bitmap(path);
	if (!bitmap)
		return nullptr;
	bitmap = convert_to_32bit_bitmap(bitmap.get());
	if (!bitmap)
		return nullptr;
	auto ret = std::unique_ptr<Image>(new Image);
	FreeImageInitializer fii;
	auto w = FreeImage_GetWidth(bitmap.get());
	auto h = FreeImage_GetHeight(bitmap.get());
	ret->w = w + (Tile::size - w % Tile::size) % Tile::size;
	ret->h = h + (Tile::size - h % Tile::size) % Tile::size;
	ret->bitmap.resize(ret->w * ret->h, color0);

	auto dst = &(ret->bitmap[0]);
	for (unsigned y = 0; y < h; y++){
		auto scanline = (pixel *)FreeImage_GetScanLine(bitmap.get(), h - 1 - y);
		for (unsigned x = 0; x < w; x++){
			dst->r = scanline->b;
			dst->g = scanline->g;
			dst->b = scanline->r;
			dst->a = scanline->a;
			dst++;
			scanline++;
		}
	}
	return ret;
}

std::set<pixel> Image::get_unique_colors() const{
	return ::get_unique_colors(this->bitmap);
}

std::unique_ptr<std::vector<byte_t>> Image::reduce(unsigned &bits) const{
	const auto unique = this->get_unique_colors();
	std::unique_ptr<std::vector<byte_t>> ret(new std::vector<byte_t>);
	bool force_four = false;
	if (unique.size() == 1){
		if (unique.find(color0) != unique.end()){
			ret->resize(this->bitmap.size(), 0);
			bits = 1;
			return ret;
		}
		if (unique.find(color3) != unique.end()){
			ret->resize(this->bitmap.size(), 0xFF);
			bits = 1;
			return ret;
		}
		if (unique.find(color1) == unique.end() && unique.find(color2) == unique.end())
			return nullptr;
		force_four = true;
	}
	if (!force_four && unique.size() == 2){
		auto count = (unique.find(color0) != unique.end()) + (unique.find(color3) != unique.end());
		if (count == unique.size()){
			unsigned bit = 0;
			for (auto &p : this->bitmap){
				unsigned value = p == color3;
				if (!bit)
					ret->push_back(value);
				else
					ret->back() |= value << bit;
				bit = (bit + 1) % 8;
			}
			bits = 1;
			return ret;
		}
	}
	if (force_four || unique.size() <= 4){
		auto count = (unique.find(color0) != unique.end()) + (unique.find(color3) != unique.end()) + (unique.find(color1) != unique.end()) + (unique.find(color2) != unique.end());
		if (!force_four && count != unique.size())
			return nullptr;
		unsigned bit = 0;
		for (auto &p : this->bitmap){
			unsigned value;
			if (p == color0)
				value = 0;
			else if (p == color1)
				value = 1;
			else if (p == color2)
				value = 2;
			else
				value = 3;
			if (!bit)
				ret->push_back(value);
			else
				ret->back() |= value << bit;
			bit = (bit + 2) % 8;
		}
		bits = 2;
		return ret;
	}
	return nullptr;
}

std::unique_ptr<Image> Image::pad_out_pic(unsigned target_size) const{
	std::unique_ptr<Image> ret(new Image(target_size * Tile::size, target_size * Tile::size));
	std::fill(ret->bitmap.begin(), ret->bitmap.end(), color0);
	unsigned tiles_w = this->w / Tile::size;
	unsigned tiles_h = this->h / Tile::size;
	assert(tiles_w <= target_size && tiles_h <= target_size);
	unsigned y0 = target_size - tiles_h;
	unsigned x0 = (target_size + 1 - tiles_w) / 2;
	for (unsigned dst_y = y0; dst_y < target_size; dst_y++){
		unsigned src_y = dst_y - y0;
		assert(src_y < tiles_h);
		for (unsigned dst_x = x0; dst_x < x0 + tiles_w; dst_x++){
			auto src_x = dst_x - x0;
			auto src = this->bitmap.begin() + (src_x * Tile::size + src_y * tiles_w * (Tile::size * Tile::size));
			auto dst = ret->bitmap.begin() + (dst_x * Tile::size + dst_y * target_size * (Tile::size * Tile::size));
			for (unsigned i = 0; i < Tile::size; i++){
				auto src2 = src + this->w * i;
				std::copy(src2, src2 + Tile::size, dst + ret->w * i);
			}
		}
	}
	return ret;
}

std::unique_ptr<Image> Image::double_size() const{
	std::unique_ptr<Image> ret(new Image(this->w * 2, this->h * 2));
	for (unsigned y = 0; y < this->h; y++){
		for (unsigned x = 0; x < this->w; x++){
			auto pix = this->bitmap[x + y * this->w];
			auto dst = ret->bitmap.begin() + (x * 2 + y * 2 * ret->w);
			dst[0] = pix;
			dst[1] = pix;
			dst[ret->w] = pix;
			dst[ret->w + 1] = pix;
		}
	}
	return ret;
}

std::vector<Tile> Image::reorder_into_tiles() const{
	std::vector<Tile> ret;
	unsigned tiles_w = this->w / Tile::size;
	unsigned tiles_h = this->h / Tile::size;
	ret.reserve(tiles_w * tiles_h);
	for (unsigned y = 0; y < tiles_h; y++){
		for (unsigned x = 0; x < tiles_w; x++){
			ret.resize(ret.size() + 1);
			auto &dst = ret.back();
			auto src = this->bitmap.begin() + (x * Tile::size + y * tiles_w * (Tile::size * Tile::size));
			for (int i = 0; i < Tile::size; i++){
				auto src2 = src + this->w * i;
				std::copy(src2, src2 + Tile::size, dst.pixels + i * Tile::size);
			}
		}
	}
	return ret;
}

void save_png(const char *path, const pixel *data, int w, int h){
	FreeImageInitializer fii;
	auto img = Image::to_ptr(FreeImage_Allocate(w, h, 32));
	for (int y = 0; y < h; y++){
		auto dst = (pixel *)FreeImage_GetScanLine(img.get(), h - 1 - y);
		auto src = data + y * w;
		for (int x = 0; x < w; x++){
			dst->b = src->r;
			dst->g = src->g;
			dst->r = src->b;
			dst->a = src->a;
			dst++;
			src++;
		}
	}
	FreeImage_Save(FIF_PNG, img.get(), path);
}
