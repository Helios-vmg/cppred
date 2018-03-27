#include "stdafx.h"
#include "ScriptDeclarations.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "../../../CodeGeneration/output/variables.h"

namespace CppRed{
namespace Scripts{

DECLARE_SCRIPT(PrintRedSNESText){
	parameters.game->run_dialogue(TextResourceId::RedBedroomSNESText, true, true);
}

DECLARE_SCRIPT(RedsHouse1FText1){
	if (parameters.game->get_variable_store().get(IntegerVariableId::event_received_starter)){
		throw std::runtime_error("Not implemented.");
	}else
		parameters.game->run_dialogue(TextResourceId::MomWakeUpText, true, true);
}

DECLARE_SCRIPT(RedsHouse1FText2){
	auto id = parameters.caller->get_facing_direction() == FacingDirection::Up ? TextResourceId::StandByMeText : TextResourceId::TVWrongSideText;
	parameters.game->run_dialogue(id, true, true);
}

DECLARE_SCRIPT(OpenRedsPC){
	auto &game = *parameters.game;
	auto old = game.get_no_text_delay();
	game.set_no_text_delay(true);
	auto &audio = game.get_audio_interface();
	audio.play_sound(AudioResourceId::SFX_Turn_On_PC);
	game.run_dialogue(TextResourceId::TurnedOnPC1Text, false, false);
	game.reset_dialogue_state(false);
	parameters.game->get_world().get_pc().open_pc(true);
	game.reset_dialogue_state();
	audio.stop_sfx();
	audio.play_sound(AudioResourceId::SFX_Turn_Off_PC);
	game.set_no_text_delay(old);
}

}
}