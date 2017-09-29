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
	Bcd,
};

class TextResourceCommand{
public:
	virtual ~TextResourceCommand(){}
	virtual void execute(CppRedEngine &, TextResource &) = 0;
};

class TextCommand : public TextResourceCommand{
	std::vector<byte_t> data;
public:
	TextCommand(const byte_t *, size_t);
	void execute(CppRedEngine &, TextResource &) override;
};

class LineCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class NextCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class ContCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class ParaCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class PageCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class PromptCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class DoneCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class DexCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class AutocontCommand : public TextResourceCommand{
public:
	void execute(CppRedEngine &, TextResource &) override;
};

class MemCommand : public TextResourceCommand{
	std::string variable;
public:
	MemCommand(const char *s): variable(s){}
	void execute(CppRedEngine &, TextResource &) override;
};

class NumCommand : public TextResourceCommand{
	std::string variable;
	int first_parameter;
	int second_parameter;
public:
	NumCommand(const char *s, int a, int b):
		variable(s),
		first_parameter(a),
		second_parameter(a){}
	void execute(CppRedEngine &, TextResource &) override;
};

class TextResource{
public:
	TextResourceId id;
	std::vector<std::unique_ptr<TextResourceCommand>> commands;

	void execute(CppRedEngine &, TextStore &);
};

class TextStore{
	std::vector<std::unique_ptr<TextResource>> resources;

	void parse_command(const byte_t *, size_t);
public:
	TextStore();
};
