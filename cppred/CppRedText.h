#pragma once
#include "CommonTypes.h"
#include "CppRedRam.h"
#include <iostream>
#include <memory>
#include <vector>
#include <map>

class CppRed;

namespace SpecialCharacters{
static const byte_t terminator = 0x50;

static const byte_t box_top_left     = 0x79;
static const byte_t box_horizontal   = 0x7A;
static const byte_t box_top_right    = 0x7B;
static const byte_t box_vertical     = 0x7C;
static const byte_t box_bottom_left  = 0x7D;
static const byte_t box_bottom_right = 0x7E;

static const byte_t arrow_white_right = 0xEC;
static const byte_t arrow_black_right = 0xED;
static const byte_t arrow_black_up    = 0xED;
static const byte_t arrow_black_down  = 0xEE;
}

std::string process_escaped_text(const char *);

class CppRedText{
	CppRed *parent;
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
		TextCommand(const char *s);
		CommandType type() const override{
			return CommandType::Text;
		}
		const std::string &get_text() const{
			return this->text;
		}
	};

	class LineCommand : public Command{
	public:
		LineCommand(){}
		CommandType type() const override{
			return CommandType::Line;
		}
	};

	class NextCommand : public Command{
	public:
		NextCommand(){}
		CommandType type() const override{
			return CommandType::Next;
		}
	};

	class ContCommand : public Command{
	public:
		ContCommand(){}
		CommandType type() const override{
			return CommandType::Cont;
		}
	};

	class PageCommand : public Command{
	public:
		PageCommand(){}
		CommandType type() const override{
			return CommandType::Page;
		}
	};

	class ParaCommand : public Command{
	public:
		ParaCommand(){}
		CommandType type() const override{
			return CommandType::Para;
		}
	};

	class DoneCommand : public Command{
	public:
		DoneCommand(){}
		CommandType type() const override{
			return CommandType::Done;
		}
	};

	class PromptCommand : public Command{
	public:
		PromptCommand(){}
		CommandType type() const override{
			return CommandType::Prompt;
		}
	};

	class DexCommand : public Command{
	public:
		DexCommand(){}
		CommandType type() const override{
			return CommandType::Dex;
		}
	};

	class AutocontCommand : public Command{
	public:
		AutocontCommand(){}
		CommandType type() const override{
			return CommandType::Autocont;
		}
	};

	class MemCommand : public Command{
		std::function<std::string()> f;
	public:
		MemCommand(std::function<std::string()> &&f): f(std::move(f)){}
		CommandType type() const override{
			return CommandType::Mem;
		}
		std::string get_string() const{
			return this->f();
		}
	};

	class NumCommand : public Command{
		NumSource source;
		int bytes_to_read;
		int digits_to_display;
	public:
		NumCommand(decltype(source) source, int bytes_to_read, int digits_to_display):
			source(source),
			bytes_to_read(bytes_to_read),
			digits_to_display(digits_to_display){}
		CommandType type() const override{
			return CommandType::Num;
		}
		const decltype(source) &get_source() const{
			return this->source;
		}
		int get_bytes_to_read() const{
			return this->bytes_to_read;
		}
		int get_digits_to_display() const{
			return this->digits_to_display;
		}
	};

	class BcdCommand : public Command{
		BcdSource source;
		unsigned flags;
	public:
		BcdCommand(decltype(source) source, unsigned flags):
			source(source),
			flags(flags){}
		CommandType type() const override{
			return CommandType::Bcd;
		}
		const decltype(source) &get_source() const{
			return this->source;
		}
	};

	class Region{
		std::vector<std::unique_ptr<Command>> commands;
	public:
		Region &operator<<(const char *s);
		Region &operator<<(std::unique_ptr<Command> &&);
		const decltype(commands) &get_commands() const{
			return this->commands;
		}
	};

#define DECLSEQ(x) Region x
#include "../CodeGeneration/output/text.h"

	CppRedText(CppRed &parent);
	typedef decltype(WRam::wTileMap)::iterator tilemap_it;
	void text_box_border(const tilemap_it &it, unsigned w, unsigned h);
	void place_string(const tilemap_it &it, const std::string &text);
	void place_string(const tilemap_it &it, const Region &text);
	void print_text(const Region &text, bool without_textbox = false);
	void scroll_text_up_one_line();

private:
	void initialize_maps();
	void text_command_processor(const Region &text, const tilemap_it &it);
	void print_letter_delay();
	void manual_text_scroll();
	void place_arrow();
	tilemap_it get_arrow_location();
	void place_blank();
	void advance_page_in_text_window(unsigned page_height, tilemap_it &it);

	typedef void (CppRedText::*command_processor_f)(const Command &, tilemap_it &, tilemap_it &);
	typedef void (CppRedText::*special_character_processor_f)(tilemap_it &);

	std::map<CommandType, command_processor_f> command_processor_map;
	special_character_processor_f special_character_processors[7];

#define DECLARE_COMMAND_PROCESSOR(name) void process_##name##_command(const Command &, tilemap_it &, tilemap_it &)
	DECLARE_COMMAND_PROCESSOR(text);
	DECLARE_COMMAND_PROCESSOR(line);
	DECLARE_COMMAND_PROCESSOR(next);
	DECLARE_COMMAND_PROCESSOR(cont);
	DECLARE_COMMAND_PROCESSOR(page);
	DECLARE_COMMAND_PROCESSOR(para);
	DECLARE_COMMAND_PROCESSOR(done);
	DECLARE_COMMAND_PROCESSOR(prompt);
	DECLARE_COMMAND_PROCESSOR(dex);
	DECLARE_COMMAND_PROCESSOR(autocont);
	DECLARE_COMMAND_PROCESSOR(mem);
	DECLARE_COMMAND_PROCESSOR(num);
	DECLARE_COMMAND_PROCESSOR(bcd);
	void process_raw_text(const std::string &string, tilemap_it &it, bool transform);
	void process_raw_text(const void *string, size_t n, tilemap_it &it, bool transform);

	void special_character_processor_0x01(tilemap_it &);
	void special_character_processor_POKE(tilemap_it &);
	void special_character_processor_pkmn(tilemap_it &);
	void special_character_processor_player(tilemap_it &);
	void special_character_processor_rival(tilemap_it &);
	void special_character_processor_user(tilemap_it &);
	void special_character_processor_target(tilemap_it &);
	void special_character_processor_pokemon_name(tilemap_it &, bool is_enemy);
};
