#include "Trainer.h"

namespace CppRed{

NpcTrainer::NpcTrainer(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &instance):
		Npc(game, parent_coroutine, name, renderer, sprite, instance){
}

}
