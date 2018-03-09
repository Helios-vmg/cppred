#pragma once

#include "ScreenOwner.h"

enum class TextResourceId;

namespace CppRed{

class TextDisplay : public ScreenOwner{
	TextResourceId text_id;
public:
	TextDisplay(Game &, TextResourceId);
	std::unique_ptr<ScreenOwner> run() override;
};

}
