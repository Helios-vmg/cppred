#include "CppRedText.h"
#include "CppRed.h"

#include "../CodeGeneration/output/charmap.inl"

const CppRedText::border_tiles_t CppRedText::trade_center_border_tiles = { 0x78, 0x79, 0x7A, 0x7B, 0x77, 0x7C, 0x76, 0x7D };
const CppRedText::border_tiles_t CppRedText::default_border_tiles = {
	SpecialCharacters::box_top_left,
	SpecialCharacters::box_horizontal,
	SpecialCharacters::box_top_right,
	SpecialCharacters::box_vertical,
	SpecialCharacters::box_vertical,
	SpecialCharacters::box_bottom_left,
	SpecialCharacters::box_horizontal,
	SpecialCharacters::box_bottom_right,
};

unsigned hex2value(char c){
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'A')
		return c - 'A' + 10;
	return c - 'a' + 10;
}

std::string process_escaped_text(const char *s){
	std::string ret;
	unsigned escape = 0;
	unsigned accum = 0;
	for (; *s; s++){
		char c = *s;
		switch (escape){
			case 0:
				if (c == '\\')
					escape = 1;
				else
					ret.push_back(c);
				break;
			case 1:
				accum = hex2value(c);
				accum <<= 4;
				escape = 2;
				break;
			case 2:
				accum |= hex2value(c);
				ret.push_back(accum);
				escape = 0;
				break;
		}
	}
	return ret;
}

CppRedText::TextCommand::TextCommand(const char *s): text(s){
	this->text = process_escaped_text(s);
}

CppRedText::Command::~Command(){
}

void CppRedText::initialize_maps(){
#define MAP_LINE_PROCESSOR(type, processor) this->command_processor_map[CommandType::type] = &CppRedText::process_##processor##_command
	MAP_LINE_PROCESSOR(Text, text);
	MAP_LINE_PROCESSOR(Line, line);
	MAP_LINE_PROCESSOR(Next, next);
	MAP_LINE_PROCESSOR(Cont, cont);
	MAP_LINE_PROCESSOR(Page, page);
	MAP_LINE_PROCESSOR(Para, para);
	MAP_LINE_PROCESSOR(Done, done);
	MAP_LINE_PROCESSOR(Prompt, prompt);
	MAP_LINE_PROCESSOR(Dex, dex);
	MAP_LINE_PROCESSOR(Autocont, autocont);
	MAP_LINE_PROCESSOR(Mem, mem);
	MAP_LINE_PROCESSOR(Num, num);
	MAP_LINE_PROCESSOR(Bcd, bcd);

	this->special_character_processors[0]  = &CppRedText::special_character_processor_0x01;
	this->special_character_processors[1]  = &CppRedText::special_character_processor_POKE;
	this->special_character_processors[2]  = &CppRedText::special_character_processor_pkmn;
	this->special_character_processors[3]  = &CppRedText::special_character_processor_player;
	this->special_character_processors[4]  = &CppRedText::special_character_processor_rival;
	this->special_character_processors[5]  = &CppRedText::special_character_processor_user;
	this->special_character_processors[6]  = &CppRedText::special_character_processor_target;
}

#define DEFINE_COMMAND_PROCESSOR(name) void CppRedText::process_##name##_command(const Command &command, tilemap_it &saved, tilemap_it &it)

DEFINE_COMMAND_PROCESSOR(text){
	auto &text_command = static_cast<const TextCommand &>(command);
	this->process_raw_text(text_command.get_text(), it, true);
}

DEFINE_COMMAND_PROCESSOR(line){
	it = this->parent->get_tilemap_location(1, 16);
}

DEFINE_COMMAND_PROCESSOR(next){
	unsigned addend;
	if (!this->parent->hram.hFlags_0xFFF6.get_unknown2())
		//Go down two lines.
		addend = 2 * tilemap_width; 
	else
		//Go down one line.
		addend = tilemap_width;
	saved += addend;
	it = saved;
}

CppRedText::tilemap_it CppRedText::get_arrow_location(){
	return this->parent->get_tilemap_location(18, 16);
}

void CppRedText::place_arrow(){
	*this->get_arrow_location() = SpecialCharacters::arrow_black_down;
}

void CppRedText::place_blank(){
	*this->get_arrow_location() = character_map_forward[' '];
}

DEFINE_COMMAND_PROCESSOR(cont){
	this->place_arrow();
	this->parent->delay3();
	this->manual_text_scroll();
	*this->parent->get_tilemap_location(18, 16) = character_map_forward[' '];
	this->process_autocont_command(command, saved, it);
}

void CppRedText::advance_page_in_text_window(unsigned page_height, tilemap_it &it){
	this->place_arrow();
	this->parent->delay3();
	this->manual_text_scroll();
	this->parent->clear_screen_area(18, page_height, this->parent->get_tilemap_location(1, 17 - page_height));
	this->parent->delay_frames(20);
	it = this->parent->get_tilemap_location(1, 18 - page_height);
}

DEFINE_COMMAND_PROCESSOR(page){
	this->advance_page_in_text_window(7, it);
}

DEFINE_COMMAND_PROCESSOR(para){
	this->advance_page_in_text_window(4, it);
}

DEFINE_COMMAND_PROCESSOR(done){
}

DEFINE_COMMAND_PROCESSOR(prompt){
	if (this->parent->wram.wLinkState.enum_value() != LinkState::Battling)
		this->place_arrow();
	this->parent->delay3();
	this->manual_text_scroll();
	this->place_blank();
	this->process_done_command(command, saved, it);
}

DEFINE_COMMAND_PROCESSOR(dex){
	*it = character_map_forward['.'];
}

DEFINE_COMMAND_PROCESSOR(autocont){
	this->scroll_text_up_one_line();
	this->scroll_text_up_one_line();
	it = this->parent->get_tilemap_location(1, 16);
}

DEFINE_COMMAND_PROCESSOR(mem){
	auto mem_command = static_cast<const MemCommand &>(command);
	auto value = mem_command.get_string();
	this->process_raw_text(value, it, true);
}

template <typename T, size_t N>
std::uint32_t read_number(const CppRedText::NumCommand &num_command, const IntegerWrapper<T, N> &variable){
	if (variable.size < num_command.get_bytes_to_read())
		throw std::runtime_error("Invalid num text command.");
	static const std::uint32_t masks[] = {
		0xFF,
		0xFFFF,
		0xFFFFFF,
		0xFFFFFFFF,
	};
	return variable & masks[num_command.get_bytes_to_read() - 2];
}

#define NUM_HCASE(x) case NumSource::x: value = read_number(num_command, this->parent->hram.x); break;
#define NUM_WCASE(x) case NumSource::x: value = read_number(num_command, this->parent->wram.x); break;

DEFINE_COMMAND_PROCESSOR(num){
	auto num_command = static_cast<const NumCommand &>(command);
	std::uint32_t value = 0;
	switch (num_command.get_source()){
		NUM_HCASE(hDexRatingNumMonsOwned);
		NUM_HCASE(hDexRatingNumMonsSeen);
		NUM_HCASE(hOaksAideNumMonsOwned);
		NUM_HCASE(hOaksAideRequirement);
		NUM_HCASE(hSpriteIndexOrTextID);
		NUM_WCASE(wCurEnemyLVL);
		NUM_WCASE(wDayCareNumLevelsGrown);
		NUM_WCASE(wDexRatingNumMonsOwned);
		NUM_WCASE(wDexRatingNumMonsSeen);
		NUM_WCASE(wEnemyNumHits);
		NUM_WCASE(wExpAmountGained);
		NUM_WCASE(wHPBarHPDifference);
		NUM_WCASE(wPlayerNumHits);
		default:
			throw std::runtime_error("CppRedText::process_num_command(): Invalid switch value.");
	}
	std::stringstream stream;
	stream << value;
	this->process_raw_text(stream.str(), it, true);
}

#define BCD_HCASE(x) case BcdSource::x: value = this->parent->hram.x; break;
#define BCD_WCASE(x) case BcdSource::x: value = this->parent->wram.x; break;
#define BCD_WCASE_MAIN(x) case BcdSource::x: value = this->parent->wram.wMainData.x; break;

DEFINE_COMMAND_PROCESSOR(bcd){
	auto bcd_command = static_cast<const BcdCommand &>(command);
	std::uint32_t value = 0;
	switch (bcd_command.get_source()){
		BCD_HCASE(hCoins);
		BCD_HCASE(hMoney);
		BCD_WCASE(wAmountMoneyWon);
		BCD_WCASE(wDayCareTotalCost);
		BCD_WCASE_MAIN(wPlayerCoins);
		BCD_WCASE(wTotalPayDayMoney);
		default:
			throw std::runtime_error("CppRedText::process_bcd_command(): Invalid switch value.");
	}
	std::stringstream stream;
	stream << value;
	this->process_raw_text(stream.str(), it, true);
}

CppRedText::Region &CppRedText::Region::operator<<(const char *s){
	this->commands.emplace_back(new TextCommand(s));
	return *this;
}

CppRedText::Region &CppRedText::Region::operator<<(std::unique_ptr<Command> &&p){
	this->commands.emplace_back(std::move(p));
	return *this;
}

void CppRedText::print_text(const Region &text, bool without_textbox){
	if (!without_textbox){
		this->parent->wram.wTextBoxID = TextBoxId::MessageBox;
		this->parent->display_textbox_id();
		this->parent->update_sprites();
		this->parent->delay3();
	}
	this->text_command_processor(text, this->parent->get_tilemap_location(1, 14));
}

void CppRedText::text_command_processor(const Region &text, const tilemap_it &it){
	auto &wram = this->parent->wram;
	byte_t old_flags = wram.wMainData.wLetterPrintingDelayFlags.get_raw_value();
	wram.wMainData.wLetterPrintingDelayFlags.set_raw_value(this->parent->hram.hPokedexTextFlags.get_raw_value());
	this->parent->wram.wTextDest = it.address();
	this->place_string(it, text);
	wram.wMainData.wLetterPrintingDelayFlags.set_raw_value(old_flags);
}

void CppRedText::place_string(const tilemap_it &it, const std::string &text){
	Region region;
	region << text.c_str();
	this->place_string(it, region);
}

void CppRedText::place_string(const tilemap_it &it, const Region &text){
	auto &wram = this->parent->wram;
	auto writing_location = it;
	auto saved_location = writing_location;
	for (auto &cmd : text.get_commands()){
		auto i = this->command_processor_map.find(cmd->type());
		if (i == this->command_processor_map.end())
			throw std::runtime_error("CppRedText::text_command_processor(): invalid switch value.");
		(this->*i->second)(*cmd, saved_location, writing_location);
	}
}

void CppRedText::process_raw_text(const std::string &string, tilemap_it &it, bool transform){
	this->process_raw_text(string.c_str(), string.size(), it, true);
}

void CppRedText::process_raw_text(const void *void_string, size_t n, tilemap_it &it, bool transform){
	auto string = (const byte_t *)void_string;
	for (size_t i = 0; i < n; i++){
		byte_t c = string[i];
		if (transform){
			if (c <= 7){
				(this->*this->special_character_processors[c - 1])(it);
				continue;
			}
			auto new_c = character_map_forward[c];
			if (!new_c){
				std::stringstream stream;
				stream << "CppRedText::process_raw_text(): Don't know how to map character " << (int)c;
				throw std::runtime_error(stream.str());
			}
			c = new_c;
		}
		*it++ = c;
		this->print_letter_delay();
	}
}

void CppRedText::special_character_processor_0x01(tilemap_it &){
	throw std::runtime_error("Internal error.");
}

void CppRedText::special_character_processor_POKE(tilemap_it &it){
	//"POK'E" string:
	static const byte_t string[] = { 0x8F, 0x8E, 0x8A, 0xBA };
	this->process_raw_text(string, sizeof(string), it, false);
}

void CppRedText::special_character_processor_pkmn(tilemap_it &it){
	//"pkmn" string:
	static const byte_t string[] = { 0xE1, 0xE2 };
	this->process_raw_text(string, sizeof(string), it, false);
}

void CppRedText::special_character_processor_player(tilemap_it &it){
	this->process_raw_text(this->parent->wram.wPlayerName.to_string(), it, true);
}

void CppRedText::special_character_processor_rival(tilemap_it &it){
	this->process_raw_text(this->parent->wram.wMainData.wRivalName.to_string(), it, true);
}

void CppRedText::special_character_processor_pokemon_name(tilemap_it &it, bool is_enemy){
	static const char enemy_string[] = "Enemy ";
	std::string nick;
	if (is_enemy){
		this->process_raw_text(enemy_string, sizeof(enemy_string) - 1, it, true);
		nick = this->parent->wram.wEnemyMonNick.to_string();
	}else
		nick = this->parent->wram.wBattleMonNick.to_string();
	this->process_raw_text(nick, it, true);
}

void CppRedText::special_character_processor_user(tilemap_it &it){
	this->special_character_processor_pokemon_name(it, !!this->parent->hram.H_WHOSETURN);
}

void CppRedText::special_character_processor_target(tilemap_it &it){
	this->special_character_processor_pokemon_name(it, !!!this->parent->hram.H_WHOSETURN);
}

void CppRedText::print_letter_delay(){
	auto &wram = this->parent->wram;
	auto &hram = this->parent->hram;

	if (wram.wMainData.wd730.get_no_print_delay() || !wram.wMainData.wLetterPrintingDelayFlags.get_delay_enable())
		return;

	unsigned frame_counter;
	if (wram.wMainData.wLetterPrintingDelayFlags.get_long_delay_enable())
		frame_counter = (unsigned)wram.wMainData.wOptions.get_text_speed();
	else
		frame_counter = 1;
	hram.H_FRAMECOUNTER = frame_counter;

	do{
		this->parent->joypad();
		if (hram.hJoyHeld.get_button_a() || hram.hJoyHeld.get_button_b()){
			this->parent->delay_frame();
			break;
		}
		//TODO: Add some delay here to prevent CPU burning.
	}while (hram.H_FRAMECOUNTER);
}

void CppRedText::text_box_border(const tilemap_it &it, unsigned w, unsigned h, const border_tiles_t &tiles){
	this->text_box_border_internal(it, w, h, tiles);
}

void CppRedText::text_box_border_internal(const tilemap_it &it0, unsigned w, unsigned h, const border_tiles_t &tiles){
	const byte_t blank = character_map_forward[' '];
	auto it = it0;
	*it++ = tiles[0];
	std::fill(it, it + w, tiles[1]);
	it[w] = tiles[2];

	for (unsigned i = 0; i < h; i++){
		it = it0 + tilemap_width * (i + 1);

		*it++ = tiles[3];
		std::fill(it, it + w, blank);
		it[w] = tiles[4];
	}

	it = it0 + tilemap_width * h;
	*it++ = tiles[5];
	std::fill(it, it + w, tiles[6]);
	it[w] = tiles[7];
}

void CppRedText::scroll_text_up_one_line(){
	const byte_t blank = character_map_forward[' '];
	for (int i = 0; i < 3; i++){
		auto dst = this->parent->get_tilemap_location(0, 13 + i);
		auto src = dst + tilemap_width;
		std::copy(src, src + tilemap_width, dst);
	}
	auto last_row = this->parent->get_tilemap_location(1, 16);
	std::fill(last_row, last_row + (tilemap_width - 2), blank);

	//Wait five frames.
	for (int i = 5; i--;)
		this->parent->delay_frame();
}

void CppRedText::manual_text_scroll(){
	if (this->parent->wram.wLinkState.enum_value() != LinkState::Battling){
		this->parent->wait_for_text_scroll_button_press();
		this->parent->play_sound(Sound::SFX_Press_AB_1);
	}
	this->parent->delay_frames(65);
}
