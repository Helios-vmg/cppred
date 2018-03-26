#include "stdafx.h"
#include "Sprite.h"
#include "Renderer.h"

Sprite::Sprite(Renderer &owner, int w, int h){
	if (w <= 0 || h <= 0)
		throw std::runtime_error("Sprite::Sprite(): Size must be >= 1x1");
	this->owner = &owner;
	this->id = owner.get_id();
	this->y = this->x = 0;
	this->w = w;
	this->h = h;
	this->tiles.resize(w * h);

	SpriteTile tile;
	tile.tile_no = 0;
	tile.flipped_x = false;
	tile.flipped_y = false;
	tile.has_priority = false;
	fill(this->tiles, tile);
}

Sprite::~Sprite(){
	if (this->owner)
		this->owner->release_sprite(this->id);
}

SpriteTile &Sprite::get_tile(int x, int y){
	if ((x < 0) | (y < 0) | (x >= this->w) | (y >= this->h))
		throw std::runtime_error("Invalid coordinates.");
	return this->tiles[x + y * this->w];
}
