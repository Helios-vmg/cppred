#pragma once
#include "Actor.h"

namespace CppRed{

class Npc : public Actor{
protected:
	void coroutine_entry_point() override;
public:
	Npc(Game &game, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite);
};

}
