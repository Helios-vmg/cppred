#include "Scripts.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "Events.h"

namespace CppRed{
namespace Scripts{

DECLARE_SCRIPT(PrintRedSNESText){
	game.run_dialog_from_world(TextResourceId::RedBedroomSNESText, caller);
}

DECLARE_SCRIPT(RedsHouse1FText1){
	if (game.get_variable_store().get_number_default(event_received_starter)){
		throw std::runtime_error("Not implemented.");
	}else
		game.run_dialog_from_world(TextResourceId::MomWakeUpText, caller);
}

DECLARE_SCRIPT(RedsHouse1FText2){
	auto id = caller.get_facing_direction() == FacingDirection::Up ? TextResourceId::StandByMeText : TextResourceId::TVWrongSideText;
	game.run_dialog_from_world(id, caller);
}

}
}