#include "Scripts.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "../../../CodeGeneration/output/audio.h"
#include "Events.h"
#include <CppRed/Npc.h>

namespace CppRed{
namespace Scripts{

static const char * const PalletTownScriptIndex = "PalletTownScriptIndex";
static const char * const wcf0d = "wcf0d";
using script_f = ScriptStore::script_f;

DECLARE_SCRIPT(PalletTownScript0);
DECLARE_SCRIPT(PalletTownScript1);
DECLARE_SCRIPT(PalletTownScript2);
DECLARE_SCRIPT(PalletTownScript3);
DECLARE_SCRIPT(PalletTownScript4);
DECLARE_SCRIPT(PalletTownScript5);
DECLARE_SCRIPT(PalletTownScript6);

DECLARE_SCRIPT(PalletTown_onload){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	auto &oak = game.get_world().get_actor("PalletOak");
	oak.set_visible(false);
}

DECLARE_SCRIPT(PalletTownScript){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get_number_default(event_got_pokeballs_from_oak))
		vs.set_number(event_pallet_after_getting_pokeballs, 1);
	auto index = vs.get_number_default(PalletTownScriptIndex);
	static const script_f scripts[] = {
		PalletTownScript0,
		PalletTownScript1,
		PalletTownScript2,
		PalletTownScript3,
		PalletTownScript4,
		PalletTownScript5,
		PalletTownScript6,
	};
	if (index >= 0 && index < array_length(scripts))
		scripts[index](parameters);
}

DECLARE_SCRIPT(PalletTownScript0){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get_number_default(event_followed_oak_into_lab))
		return;
	auto &player = game.get_world().get_pc();
	auto position = player.get_map_position();
	if (position.y > 1)
		return;
	player.set_facing_direction(FacingDirection::Down);
	auto &ai = game.get_audio_interface();
	ai.play_sound(AudioResourceId::Stop);
	ai.play_sound(AudioResourceId::Music_MeetProfOak);
	vs.set_number(event_oak_appeared_in_pallet, 1);
	vs.set_number(PalletTownScriptIndex, 1);
	PalletTownScript1(parameters);
}

DECLARE_SCRIPT(PalletTownScript1){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	vs.set_number(wcf0d, 0);
	auto &world = game.get_world();
	auto &player = world.get_pc();
	player.set_ignore_input(true);
	game.get_engine().set_gamepad_disabled(true);
	auto &renderer = game.get_engine().get_renderer();
	game.run_dialog(TextResourceId::OakAppearsText, TileRegion::Window);
	auto &coroutine = Coroutine::get_current_coroutine();
	{
		auto exclamation_mark = renderer.create_sprite(2, 2);
		int i = 0;
		for (SpriteTile &tile : exclamation_mark->iterate_tiles()){
			tile.tile_no = EmotionBubbles.first_tile + i++;
			tile.has_priority = true;
		}
		exclamation_mark->set_position((PlayerCharacter::screen_block_offset + Point(0, -1)) * Renderer::tile_size * 2 + Point(0, -Renderer::tile_size / 2));
		exclamation_mark->set_visible(true);
		coroutine.wait(1);
		renderer.set_enable_window(false);
		game.reset_dialog_state();
	}
	auto &oak = world.get_actor("PalletOak");
	oak.set_visible(true);
	oak.set_facing_direction(FacingDirection::Up);
	for (int i = 0; i < 5; i++)
		oak.move(!(i % 2) ? FacingDirection::Up : FacingDirection::Right);
	game.get_engine().set_gamepad_disabled(false);
	game.run_dialog(TextResourceId::OakWalksUpText, TileRegion::Window, true);
	game.reset_dialog_state();
	renderer.set_enable_window(false);
	player.set_ignore_input(false);
	{
		auto old = oak.movement_duration();
		static_cast<CppRed::Npc &>(oak).set_special_movement_duration(player.movement_duration());
		Point destination(12, 11);
		world.get_map_instance(oak.get_current_map()).set_cell_occupation(oak.get_map_position(), false);
		auto path1 = oak.find_path(destination);
		auto path2 = player.find_path(destination);
		bool done1 = false, done2 = false;
		Coroutine player_co("player temp coroutine", Coroutine::get_current_coroutine().get_clock(), [&](Coroutine &){
			player.follow_path(path2);
			done2 = true;
		});
		Coroutine oak_co("player temp coroutine", Coroutine::get_current_coroutine().get_clock(), [&](Coroutine &){
			oak.follow_path(path1);
			done1 = true;
		});
		while (!done1 || !done2){
			player_co.get_clock().step();
			player_co.resume();
			oak_co.get_clock().step();
			oak_co.resume();
			coroutine.yield();
		}
		static_cast<CppRed::Npc &>(oak).set_special_movement_duration(old);
	}
	vs.set_number(PalletTownScriptIndex, -1);
}

DECLARE_SCRIPT(PalletTownScript2){
}

DECLARE_SCRIPT(PalletTownScript3){
}

DECLARE_SCRIPT(PalletTownScript4){
}

DECLARE_SCRIPT(PalletTownScript5){
}

DECLARE_SCRIPT(PalletTownScript6){
}

}
}
