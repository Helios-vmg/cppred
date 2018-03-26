#include "ItemActor.h"
#include "Game.h"
#include "World.h"

namespace CppRed{

ItemActor::ItemActor(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &instance):
		NonPlayerActor(game, parent_coroutine, name, renderer, sprite, instance){
}

ItemActor::~ItemActor(){}

}
