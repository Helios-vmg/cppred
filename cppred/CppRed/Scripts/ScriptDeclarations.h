#pragma once

#include "Scripts.h"

#define DECLARE_SCRIPT(name) void name(const script_parameters &parameters)

namespace CppRed{
namespace Scripts{

DECLARE_SCRIPT(PrintRedSNESText);
DECLARE_SCRIPT(RedsHouse1FText1);
DECLARE_SCRIPT(RedsHouse1FText2);
DECLARE_SCRIPT(PalletTownScript);
DECLARE_SCRIPT(BluesHouseScript);
DECLARE_SCRIPT(BluesHouseText1);
DECLARE_SCRIPT(OaksLabScript);
DECLARE_SCRIPT(OaksLabText1);
DECLARE_SCRIPT(OaksLabText2);
DECLARE_SCRIPT(OaksLabText3);
DECLARE_SCRIPT(OaksLabText4);
DECLARE_SCRIPT(OaksLabText5);
DECLARE_SCRIPT(DisplayOakLabEmailText);
DECLARE_SCRIPT(DisplayOakLabLeftPoster);
DECLARE_SCRIPT(DisplayOakLabRightPoster);
DECLARE_SCRIPT(OpenRedsPC);

}
}
