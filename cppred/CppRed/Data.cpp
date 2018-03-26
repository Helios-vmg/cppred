#include "stdafx.h"
#include "common_types.h"
#include "Data.h"
#include "Renderer.h"
#include "Maps.h"

#define AT_LEVEL(level, next_form) EvolutionTriggerType::Level, level, SpeciesId::next_form
#define WITH_ITEM(item, level, next_form) EvolutionTriggerType::Item, level, SpeciesId::next_form, ItemId::item
#define WHEN_TRADED(level, next_form) EvolutionTriggerType::Trade, level, SpeciesId::next_form
#define LEARN(level, move) level, MoveId::move
#define GET_TRAINER(trainer_class, index) get_trainer<index>(trainer_class)

template <int index, size_t N>
const BaseTrainerParty *get_trainer(const BaseTrainerParty * const (&array)[N]){
	static_assert(index - 1 >= 0 && index - 1 < N, "Invalid trainer index.");
	return array[index - 1];
}

#include "../CodeGeneration/output/pokemon_definitions.inl"
#include "../CodeGeneration/output/text.inl"
#include "../CodeGeneration/output/graphics.inl"
#include "../CodeGeneration/output/audio.inl"
#include "../CodeGeneration/output/trainer_parties.inl"
#include "../CodeGeneration/output/map_objects.inl"
#include "../CodeGeneration/output/maps.inl"
#include "../CodeGeneration/output/items.inl"
#include "../CodeGeneration/output/variables.inl"
