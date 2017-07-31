#include "CommonTypes.h"
#include <iostream>
#include <memory>

class CppRedText{
public:

#include "../CodeGeneration/output/text_enum.h"

	class Command{
	public:
		virtual ~Command() = 0;
	};

	class TextCommand : public Command{
		std::string text;
	public:
		TextCommand(const std::string &);
	};

	class MemCommand : public Command{
	public:
		MemCommand(MemorySources);
	};

	class LineCommand : public Command{
	public:
		LineCommand();
	};

	class NextCommand : public Command{
	public:
		NextCommand();
	};

	class ContCommand : public Command{
	public:
		ContCommand();
	};

	class PageCommand : public Command{
	public:
		PageCommand();
	};

	class ParaCommand : public Command{
	public:
		ParaCommand();
	};

	class DoneCommand : public Command{
	public:
		DoneCommand();
	};

	class PromptCommand : public Command{
	public:
		PromptCommand();
	};

	class NumCommand : public Command{
	public:
		NumCommand(MemorySources, int, int);
	};

	class BcdCommand : public Command{
	public:
		BcdCommand(MemorySources, int);
	};

	class DexCommand : public Command{
	public:
		DexCommand();
	};

	class Region{
	public:
		Region &operator<<(const char *);
		Region &operator<<(std::unique_ptr<Command> &&);
	};

#define DECLSEQ(x) Region sequence_##x
#include "../CodeGeneration/output/text.h"

	CppRedText();
};
