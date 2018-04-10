#include "stdafx.h"
#include "ItemFunctions.h"
#include "PlayerCharacter.h"
#include "Game.h"
#include "../../CodeGeneration/output/variables.h"
#include "../../CodeGeneration/output/audio.h"

namespace CppRed{

#define DEFINE_ITEM_FUNCTION(name) ItemUseResult name(const ItemData &item_used, Game &game, PlayerCharacter &user, bool from_battle)
#define DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(name) DEFINE_ITEM_FUNCTION(name){ return ItemUseResult(ItemUseError::NotImplemented); }

DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseBait)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseBall)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseBicycle)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseCardKey)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseCoinCase)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseDireHit)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseEscapeRope)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseEvoStone)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseGoodRod)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseGuardSpec)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseItemfinder)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseMaxRepel)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseMedicine)

DEFINE_ITEM_FUNCTION(ItemUsePotion){
	if (!user.get_party().size()){
		game.run_dialogue(TextResourceId::EmptyPartyText, false, true);
		return ItemUseResult(ItemUseError::EmptyParty);
	}
	auto &renderer = game.get_engine().get_renderer();
	AutoRendererPusher pusher(renderer);
	renderer.clear_screen();
	renderer.clear_windows();
	renderer.clear_sprites();
	game.reset_dialogue_state();

	PlayerCharacter::PartyMenuOptions options;
	options.prompt = TextResourceId::PartyMenuItemUseText;
	options.push_renderer = false;
	Pokemon *target = nullptr;
	int target_index = -1;
	options.callback = [&target, &target_index](Pokemon &p, int idx){
		target = &p;
		target_index = idx;
		return PlayerCharacter::InventoryChanges::Exit;
	};
	if (!user.display_party_menu(options))
		return ItemUseResult(ItemUseError::UseCancelled);

	auto hp = target->get_current_hp();
	auto max_hp = target->get_max_hp();
	
	auto &tilemap = renderer.get_tilemap(TileRegion::Background);
	std::vector<std::shared_ptr<Sprite>> sprites;
	draw_party(renderer, user.get_party(), tilemap, sprites);
	auto &arrow_tile = tilemap[{0, target_index * 2 + 1}];
	Tile arrow(white_arrow);
	arrow_tile = arrow;

	if (hp == max_hp){
		game.run_dialogue(TextResourceId::ItemUseNoEffectText, false, true);
		return ItemUseResult(ItemUseError::NoEffect);
	}

	auto target_hp = std::min(hp + item_used.function_parameter, max_hp);
	if (item_used.function_parameter < 0)
		target_hp = max_hp;

	game.get_audio_interface().play_sound(AudioResourceId::SFX_Heal_HP);
	//Arbitrary magic number determined empirically.
	static const double k = sqrt(287.0) / 40.0;
	auto hp_delta = target_hp - hp;
	auto animation_duration = sqrt((double)max_hp) * k * hp_delta / max_hp;
	auto &coroutine = Coroutine::get_current_coroutine();
	auto &clock = coroutine.get_clock();
	auto t0 = clock.get();
	auto t1 = t0 + animation_duration;
	double t;
	Point bar_position(6, target_index * 2 + 1);
	while ((t = clock.get()) < t1){
		coroutine.yield();
		auto real_hp = (t - t0) / animation_duration * hp_delta + hp;
		target->set_current_hp((int)real_hp);
		draw_party(renderer, user.get_party(), tilemap);
		//Redraw the bar for the target, to perform a smooth animation.
		renderer.draw_bar(bar_position, TileRegion::Background, 6, max_hp * 100, (int)(real_hp * 100));
		arrow_tile = arrow;
	}

	target->set_current_hp(target_hp);
	draw_party(renderer, user.get_party(), tilemap);
	arrow_tile = arrow;
	coroutine.yield();

	auto &vs = game.get_variable_store();
	vs.set(StringVariableId::wcd6d_ActionTargetMonName, target->get_display_name());
	vs.set(IntegerVariableId::wHPBarHPDifference, target_hp - hp);
	game.run_dialogue(TextResourceId::PotionText, true, true);

	return ItemUseResult();
}

DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseOaksParcel)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseOldRod)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUsePokedex)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUsePokedoll)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUsePokeflute)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUsePPRestore)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUsePPUp)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseRepel)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseRock)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseSuperRepel)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseSuperRod)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseSurfboard)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseTownMap)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseVitamin)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseXAccuracy)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseXStat)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(UnusableItem)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseHm)
DEFINE_UNIMPLEMENTED_ITEM_FUNCTION(ItemUseTm)

}
