#include "Npc.h"
#include "Game.h"

namespace CppRed{

Npc::Npc(Game &game, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite): Actor(game, name, renderer, sprite){
	
}

void Npc::coroutine_entry_point(){
	this->standing_sprites[(int)FacingDirection::Down]->set_visible(true);
	while (!this->quit_coroutine){
		auto cam = this->game->get_camera_position();
		auto po = this->game->get_pixel_offset();
		auto position = (this->position.position - cam) * (Renderer::tile_size * 2) + po;
		position.y -= Renderer::tile_size / 2;
		this->apply_to_all_sprites([&position](Sprite &sprite){ sprite.set_position(position); });
		this->coroutine->yield();
	}
}

}
