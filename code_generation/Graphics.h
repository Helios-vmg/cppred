#pragma once
#include "Image.h"
#include <map>

enum class ImageType{
	Normal = 0,
	Charmap,
	Pic,
	BackPic,
};

struct Graphic{
	std::string name;
	std::string path;
	ImageType type;
	std::vector<Tile> tiles;
	std::vector<unsigned> corrected_tile_numbers;
	int first_tile = -1;
	int w = -1, h = -1;
	std::unique_ptr<Image> image;

	Graphic() = default;
	Graphic(const Graphic &other){
		*this = other;
	}
	Graphic(Graphic &&other){
		*this = std::move(other);
	}
	const Graphic &operator=(const Graphic &other);
	const Graphic &operator=(Graphic &&other);

	bool operator<(const Graphic &other) const{
		return this->type == ImageType::Charmap && other.type != ImageType::Charmap;
	}
};

extern const char * const graphics_csv_path;

std::vector<std::shared_ptr<Graphic>> load_graphics_from_csv(const char *path);

class GraphicsStore{
	std::unique_ptr<std::vector<std::shared_ptr<Graphic>>> data;
	std::map<std::string, std::shared_ptr<Graphic>> map;
public:
	std::shared_ptr<Graphic> &get(const std::string &name);
	std::vector<std::shared_ptr<Graphic>> &get();
};
