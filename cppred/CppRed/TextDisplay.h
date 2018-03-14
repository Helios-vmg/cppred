#pragma once

#include "ScreenOwner.h"

enum class TextResourceId;

namespace CppRed{

class TextDisplay : public ScreenOwner{
	TextResourceId text_id;
	bool hide;
public:
	TextDisplay(Game &, TextResourceId, bool hide_window_at_end = true);
	std::unique_ptr<ScreenOwner> run() override;
	void pause() override{}
};

}
