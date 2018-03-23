#pragma once

#include "ScreenOwner.h"

enum class TextResourceId;

namespace CppRed{

class TextDisplay : public ScreenOwner{
	TextResourceId text_id;
	bool wait;
	bool hide;
public:
	TextDisplay(Game &, TextResourceId, bool wait_at_end, bool hide_window_at_end);
	void pause() override{}
};

}
