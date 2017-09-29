#include "CppRedData.h"
#include "Renderer.h"

#define AT_LEVEL(level, next_form) EvolutionTriggerType::Level, level, SpeciesId::next_form
#define WITH_ITEM(item, level, next_form) EvolutionTriggerType::Item, level, SpeciesId::next_form, ItemId::item
#define WHEN_TRADED(level, next_form) EvolutionTriggerType::Trade, level, SpeciesId::next_form
#define LEARN(level, move) level, MoveId::move

#include "../CodeGeneration/output/pokemon_definitions.inl"
#include "../CodeGeneration/output/graphics.inl"
#include "../CodeGeneration/output/text.inl"
