#include "Trainer.h"

namespace CppRed{

Trainer::Trainer(Game &game, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite):
		Actor(game, name, renderer, sprite){
}

}
