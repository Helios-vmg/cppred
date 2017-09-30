#pragma once
#include "CppRedData.h"
#include <vector>
#include <memory>

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

class TextResourceCommand{
public:
	virtual ~TextResourceCommand(){}
	virtual void execute(CppRedEngine &, TextStore &) = 0;
};

class TextCommand : public TextResourceCommand{
	std::vector<byte_t> data;
public:
	TextCommand(const byte_t *, size_t);
	void execute(CppRedEngine &, TextStore &) override;
};

class LineCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class NextCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class ContCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class ParaCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class PageCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class PromptCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class DoneCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class DexCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class AutocontCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextStore &) override;
};

class MemCommand : public TextResourceCommand{
	std::string variable;
public:
	MemCommand(std::string &&s): variable(std::move(s)){}
	void execute(CppRedEngine &, TextStore &) override;
};

class NumCommand : public TextResourceCommand{
	std::string variable;
	int digits;
public:
	NumCommand(std::string &&s, int digits):
		variable(std::move(s)),
		digits(digits){}
	void execute(CppRedEngine &, TextStore &) override;
};

class TextResource{
public:
	TextResourceId id;
	std::vector<std::unique_ptr<TextResourceCommand>> commands;

	void execute(CppRedEngine &, TextStore &);
};

class TextStore{
	std::vector<std::unique_ptr<TextResource>> resources;

	std::unique_ptr<TextResource> parse_resource(const byte_t *&, size_t &);
	std::unique_ptr<TextResourceCommand> parse_command(const byte_t *&, size_t &, bool &stop);
public:
	TextStore();
};
