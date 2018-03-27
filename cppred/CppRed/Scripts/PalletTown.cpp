#include "stdafx.h"
#include "ScriptDeclarations.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "../../../CodeGeneration/output/audio.h"
#include "../../../CodeGeneration/output/variables.h"
#include "../../../CodeGeneration/output/actors.h"
#include "CppRed/Npc.h"
#include "../../Coroutine.h"

namespace CppRed{
namespace Scripts{

using script_f = ScriptStore::script_f;

DECLARE_SCRIPT(PalletTownScript0);
DECLARE_SCRIPT(PalletTownScript5);

DECLARE_SCRIPT(PalletTownScript){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_got_pokeballs_from_oak))
		vs.set(EventId::event_pallet_after_getting_pokeballs, true);
	auto index = vs.get(IntegerVariableId::PalletTownScriptIndex);
	static const script_f scripts[] = {
		PalletTownScript0,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		PalletTownScript5,
	};
	if (index >= 0 && index < array_length(scripts))
		scripts[index](parameters);
}

DECLARE_SCRIPT(PalletTownScript0){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_followed_oak_into_lab))
		return;
	auto &player = game.get_world().get_pc();
	auto position = player.get_map_position();
	if (position.y > 1)
		return;
	if (player.is_moving())
		player.abort_movement();
	player.set_facing_direction(FacingDirection::Down);
	auto &ai = game.get_audio_interface();
	ai.play_sound(AudioResourceId::Stop);
	ai.play_sound(AudioResourceId::Music_MeetProfOak);
	vs.set(EventId::event_oak_appeared_in_pallet, true);
	vs.set(IntegerVariableId::PalletTownScriptIndex, 1);

	vs.set(IntegerVariableId::wcf0d, 0);
	auto &world = game.get_world();
	player.set_ignore_input(true);
	game.get_engine().set_gamepad_disabled(true);
	auto &renderer = game.get_engine().get_renderer();
	game.run_dialogue(TextResourceId::OakAppearsText, false, false);
	auto &coroutine = Coroutine::get_current_coroutine();
	{
		auto bubble = player.show_emotion_bubble(renderer, EmotionBubble::Surprise);
		coroutine.wait(1);
	}
	game.reset_dialogue_state();
	auto &oak = world.get_actor(ActorId::PalletOak);
	oak.set_visible(true);
	oak.set_facing_direction(FacingDirection::Up);
	for (int i = 0; i < 5; i++)
		oak.move(!(i % 2) ? FacingDirection::Up : FacingDirection::Right);
	if (player.get_map_position().x > 10){
		oak.move(FacingDirection::Up);
		oak.set_facing_direction(FacingDirection::Right);
		player.set_facing_direction(FacingDirection::Left);
	}
	game.get_engine().set_gamepad_disabled(false);
	game.run_dialogue(TextResourceId::OakWalksUpText, true, true);
	{
		auto old = oak.movement_duration();
		static_cast<Npc &>(oak).set_special_movement_duration(player.movement_duration());
		const Point oak_destination(12, 11);
		const Point player_destination = oak_destination + Point(0, 1);
		world.get_map_instance(oak.get_current_map()).set_cell_occupation(oak.get_map_position(), false);
		auto oak_path = oak.find_path(oak_destination);
		auto player_path = player.find_path(player_destination);
		bool oak_done = false,
			player_done = false;
		Coroutine player_co("player temp coroutine", coroutine.get_clock(), [&](Coroutine &){
			player.follow_path(player_path);
			player_done = true;
		});
		Coroutine oak_co("oak temp coroutine", coroutine.get_clock(), [&](Coroutine &){
			oak.follow_path(oak_path);
			oak.set_visible(false);
			oak_done = true;
		});
		while (!oak_done || !player_done){
			if (!player_done){
				player_co.get_clock().step();
				player_co.resume();
			}
			if (!oak_done){
				oak_co.get_clock().step();
				oak_co.resume();
			}
			coroutine.yield();
		}
		static_cast<Npc &>(oak).set_special_movement_duration(old);
	}
	player.set_ignore_occupancy(true);
	world.set_automatic_music_transition(false);
	player.move(FacingDirection::Up);
	player.set_ignore_occupancy(false);

	vs.set(IntegerVariableId::PalletTownScriptIndex, 5);
	PalletTownScript5(parameters);
}

DECLARE_SCRIPT(PalletTownScript5){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (!(vs.get(EventId::event_daisy_walking) || vs.get(EventId::event_got_town_map) && vs.get(EventId::event_entered_blues_house))){
		vs.set(EventId::event_daisy_walking, true);
		vs.set(VisibilityFlagId::hs_daisy_sitting, false);
		vs.set(VisibilityFlagId::hs_daisy_walking, true);
	}
	if (!vs.get(EventId::event_got_pokeballs_from_oak))
		return;
	vs.set(EventId::event_pallet_after_getting_pokeballs_2, true);
}

}
}
