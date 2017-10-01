#pragma once
#include "CppRedData.h"
#include <vector>
#include <memory>
#include "RendererStructs.h"

class TextResource;
class CppRedEngine;
class TextStore;

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
	void wait_for_continue(CppRedEngine &cppred, TextState &state);
public:
	virtual ~TextResourceCommand(){}
	virtual void execute(CppRedEngine &, TextState &) = 0;
};

class TextCommand : public TextResourceCommand{
	std::vector<byte_t> data;
public:
	TextCommand(const byte_t *, size_t);
	void execute(CppRedEngine &, TextState &) override;
};

class LineCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextState &) override;
};

class NextCommand : public LineCommand{
public:
};

class ContCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextState &) override;
};

class ParaCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextState &) override;
};

class PageCommand : public ParaCommand{
public:
};

class DoneCommand : public TextResourceCommand{
public:
	virtual void execute(CppRedEngine &, TextState &) override;
};

class PromptCommand : public DoneCommand{
public:
	void execute(CppRedEngine &, TextState &) override;
};

class DexCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextState &) override;
};

class AutocontCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextState &) override;
};

class MemCommand : public TextResourceCommand{
	std::string variable;
public:
	MemCommand(std::string &&s): variable(std::move(s)){}
	void execute(CppRedEngine &, TextState &) override;
};

class NumCommand : public TextResourceCommand{
	std::string variable;
	int digits;
public:
	NumCommand(std::string &&s, int digits):
		variable(std::move(s)),
		digits(digits){}
	void execute(CppRedEngine &, TextState &) override;
};

class TextResource{
public:
	TextResourceId id;
	std::vector<std::unique_ptr<TextResourceCommand>> commands;

	void execute(CppRedEngine &, TextState &);
};

class TextStore{
	std::vector<std::unique_ptr<TextResource>> resources;

	std::unique_ptr<TextResource> parse_resource(const byte_t *&, size_t &);
	std::unique_ptr<TextResourceCommand> parse_command(const byte_t *&, size_t &, bool &stop);
public:
	TextStore();
	void execute(CppRedEngine &, TextResourceId, TextState &);
};
