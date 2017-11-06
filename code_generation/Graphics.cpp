#include "Graphics.h"
#include "../common/csv_parser.h"
#include "utility.h"
#include <algorithm>

const char * const graphics_csv_path = "input/graphics.csv";

const Graphic &Graphic::operator=(const Graphic &other){
	this->name = other.name;
	this->path = other.path;
	this->type = other.type;
	this->tiles = other.tiles;
	this->corrected_tile_numbers = other.corrected_tile_numbers;
	this->first_tile = other.first_tile;
	this->w = other.w;
	this->h = other.h;
	return *this;
}

const Graphic &Graphic::operator=(Graphic &&other){
	this->name = std::move(other.name);
	this->path = std::move(other.path);
	this->type = other.type;
	this->tiles = std::move(other.tiles);
	this->corrected_tile_numbers = std::move(other.corrected_tile_numbers);
	this->first_tile = other.first_tile;
	this->w = other.w;
	this->h = other.h;
	return *this;
}

std::vector<std::shared_ptr<Graphic>> load_graphics_from_csv(const char *path){
	static const std::vector<std::string> columns = {
		"name",
		"path",
		"type",
	};
	CsvParser csv(path);
	std::vector<std::shared_ptr<Graphic>> ret;
	ret.reserve(csv.row_count());
	int first_tile = 0;

	for (size_t i = 0; i < csv.row_count(); i++){
		auto row = csv.get_ordered_row(i, columns);
		auto gr = std::make_shared<Graphic>();
		gr->name = row[0];
		if (gr->name.size() == 0)
			throw std::runtime_error("Invalid input.");
		if (gr->name[0] == '#')
			continue;
		gr->path = "input/" + row[1];
		gr->type = (ImageType)to_unsigned(row[2]);

		ret.push_back(gr);
	}

	std::sort(ret.begin(), ret.end());

	for (auto &gr : ret){
		auto image = Image::load_image(gr->path.c_str());
		if (!image)
			throw std::runtime_error("Error processing " + gr->path);

		switch (gr->type){
			case ImageType::Normal:
			case ImageType::Charmap:
				break;
			case ImageType::Pic:
				image = image->pad_out_pic(7);
				break;
			case ImageType::BackPic:
				image = image->double_size();
				break;
			default:
				throw std::runtime_error("Internal error.");
		}

		gr->first_tile = first_tile;
		gr->tiles = image->reorder_into_tiles();
		first_tile += gr->tiles.size();
		gr->w = image->w / Tile::size;
		gr->h = image->h / Tile::size;
		gr->image = std::move(image);
	}

	return ret;
}

std::vector<std::shared_ptr<Graphic>> &GraphicsStore::get(){
	if (!this->data){
		this->data = std::make_unique<std::vector<std::shared_ptr<Graphic>>>(load_graphics_from_csv(graphics_csv_path));
		this->map.clear();
		for (auto &p : *this->data)
			this->map[p->name] = p;
	}
	return *this->data;
}

std::shared_ptr<Graphic> &GraphicsStore::get(const std::string &name){
	this->get();
	auto it = this->map.find(name);
	if (it == this->map.end())
		throw std::runtime_error("Error: Invalid graphics \"" + name + "\"");
	return it->second;
}
