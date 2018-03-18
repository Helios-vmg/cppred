#pragma once
#include "Actor.h"

namespace CppRed{

class ItemActor : public NonPlayerActor{
protected:
public:
	ItemActor(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &);
	void set_facing_direction(FacingDirection direction){}
};

}
