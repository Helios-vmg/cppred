#include "CommonTypes.h"
#include "CppRedRam.h"
#include <iostream>
#include <memory>
#include <vector>

class CppRedText{
public:

#include "../CodeGeneration/output/text_enum.h"
	
	enum class CommandType{
		Text = 0,
		Line,
		Next,
		Cont,
		Page,
		Para,
		Done,
		Prompt,
		Dex,
		Autocont,
		Mem,
		Num,
		Bcd,
	};

	class Command{
	public:
		virtual ~Command() = 0;
		virtual CommandType type() const = 0;
	};

	class TextCommand : public Command{
		std::string text;
	public:
		TextCommand(const char *s): text(s){}
		CommandType type() const override{
			return CommandType::Text;
		}
	};

	class LineCommand : public Command{
	public:
		LineCommand();
		CommandType type() const override{
			return CommandType::Line;
		}
	};

	class NextCommand : public Command{
	public:
		NextCommand();
		CommandType type() const override{
			return CommandType::Next;
		}
	};

	class ContCommand : public Command{
	public:
		ContCommand();
		CommandType type() const override{
			return CommandType::Cont;
		}
	};

	class PageCommand : public Command{
	public:
		PageCommand();
		CommandType type() const override{
			return CommandType::Page;
		}
	};

	class ParaCommand : public Command{
	public:
		ParaCommand();
		CommandType type() const override{
			return CommandType::Para;
		}
	};

	class DoneCommand : public Command{
	public:
		DoneCommand();
		CommandType type() const override{
			return CommandType::Done;
		}
	};

	class PromptCommand : public Command{
	public:
		PromptCommand();
		CommandType type() const override{
			return CommandType::Prompt;
		}
	};

	class DexCommand : public Command{
	public:
		DexCommand();
		CommandType type() const override{
			return CommandType::Dex;
		}
	};

	class AutocontCommand : public Command{
	public:
		AutocontCommand();
		CommandType type() const override{
			return CommandType::Autocont;
		}
	};

	class MemCommand : public Command{
		MemorySources source;
	public:
		MemCommand(MemorySources m): source(m){}
		CommandType type() const override{
			return CommandType::Mem;
		}
	};

	class NumCommand : public Command{
		MemorySources source;
		int bytes_to_read;
		int digits_to_display;
	public:
		NumCommand(MemorySources source, int bytes_to_read, int digits_to_display):
			source(source),
			bytes_to_read(bytes_to_read),
			digits_to_display(digits_to_display){}
		CommandType type() const override{
			return CommandType::Num;
		}
	};

	class BcdCommand : public Command{
		MemorySources source;
		unsigned flags;
	public:
		BcdCommand(MemorySources source, unsigned flags):
			source(source),
			flags(flags){}
		CommandType type() const override{
			return CommandType::Bcd;
		}
	};

	class Region{
		std::vector<std::unique_ptr<Command>> commands;
	public:
		Region &operator<<(const char *s);
		Region &operator<<(std::unique_ptr<Command> &&);
	};

#define DECLSEQ(x) Region x
#include "../CodeGeneration/output/text.h"

	CppRedText();
	typedef decltype(WRam::wTileMap)::iterator tilemap_it;
	void text_box_border(const tilemap_it &it, unsigned w, unsigned h);
	void place_string(const tilemap_it &it, const Region &text);
};
