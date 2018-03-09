#pragma once
#include "Data.h"
#include "RendererStructs.h"
#include <vector>
#include <memory>
#include <string>

namespace CppRed{

class TextResource;
class TextStore;
class Game;

enum class TextResourceCommandType{
	End = 0,
	Text,
	Line,
	Next,
	Cont,
	Para,
	Page,
	Prompt,
	Done,
	Dex,
	Autocont,
	Mem,
	Num,
	Cry,
};

class TextState{
public:
	TileRegion region;
	Point start_of_line;
	Point position;
	Point box_corner,
		box_size;
	Point continue_location;
	Point first_position;
};

class TextResourceCommand{
protected:
	void wait_for_continue(Game &game, TextState &state);
public:
	virtual ~TextResourceCommand(){}
	virtual void execute(Game &, TextState &) = 0;
};

class TextCommand : public TextResourceCommand{
	std::vector<byte_t> data;
public:
	TextCommand(std::vector<byte_t> &&data): data(std::move(data)){}
	void execute(Game &, TextState &) override;
};

class LineCommand : public TextResourceCommand{
public:
	void execute(Game &, TextState &) override;
};

class NextCommand : public LineCommand{
public:
};

class ContCommand : public TextResourceCommand{
public:
	void execute(Game &, TextState &) override;
};

class ParaCommand : public TextResourceCommand{
public:
	void execute(Game &, TextState &) override;
};

class PageCommand : public ParaCommand{
public:
};

class DoneCommand : public TextResourceCommand{
public:
	virtual void execute(Game &, TextState &) override;
};

class PromptCommand : public DoneCommand{
public:
	void execute(Game &, TextState &) override;
};

class DexCommand : public TextResourceCommand{
public:
	void execute(Game &, TextState &) override;
};

class AutocontCommand : public TextResourceCommand{
public:
	void execute(Game &, TextState &) override;
};

class MemCommand : public TextResourceCommand{
	std::string variable;
public:
	MemCommand(std::string &&s): variable(std::move(s)){}
	void execute(Game &, TextState &) override;
};

class NumCommand : public TextResourceCommand{
	std::string variable;
	int digits;
public:
	NumCommand(std::string &&s, int digits):
		variable(std::move(s)),
		digits(digits){}
	void execute(Game &, TextState &) override;
};

class CryCommand : public TextResourceCommand{
	SpeciesId species;
public:
	CryCommand(SpeciesId species): species(species){}
	void execute(Game &, TextState &) override;
};

class TextResource{
public:
	TextResourceId id;
	std::vector<std::unique_ptr<TextResourceCommand>> commands;

	void execute(Game &, TextState &);
};

class TextStore{
	std::vector<std::unique_ptr<TextResource>> resources;

	std::unique_ptr<TextResource> parse_resource(const std::vector<std::pair<std::string, SpeciesId>> &species, BufferReader &);
	std::unique_ptr<TextResourceCommand> parse_command(const std::vector<std::pair<std::string, SpeciesId>> &species, BufferReader &, bool &stop);
public:
	TextStore();
	void execute(Game &, TextResourceId, TextState &);
};

}
