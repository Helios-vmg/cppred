#include "Trainer.h"

namespace CppRed{

Trainer::Trainer(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite):
		Actor(game, parent_coroutine, name, renderer, sprite){
}

}
