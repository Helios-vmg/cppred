#include "CppRedTitleScreen.h"
#include "CppRed.h"
#include "../CodeGeneration/output/gfx.h"

CppRedTitleScreen::CppRedTitleScreen(CppRed &parent):
	parent(&parent),
	wram(parent.wram),
	hram(parent.hram){
}

TitleScreenResult CppRedTitleScreen::display(){
	this->parent->gb_pal_whiteout();
	this->hram.H_AUTOBGTRANSFERENABLED = 1;
	this->hram.hTilesetType = 0;
	this->hram.hSCX = 0;
	this->hram.hSCY = 0x40;
	this->hram.hWY = 0x90;
	this->parent->clear_screen();
	this->parent->disable_lcd();
	//Note: this call makes little sense here, as most of the characters get
	//overwritten by the Pokemon logo.
	this->parent->load_font_tile_patterns();
	this->load_copyright_graphics();
	this->load_pokemon_logo();
	this->load_version_graphics();
	this->parent->clear_both_bg_maps();
	this->copy_pokemon_logo_to_wram_tilemap();
	this->draw_player_character();

	//Put a pokeball in the player's hand.
	this->wram.wOAMBuffer[10].y_position = 116;

	this->copy_copyright_to_wram_tilemap();
	this->parent->save_screen_tiles_to_buffer2();
	this->parent->load_screen_tiles_from_buffer2();
	this->parent->enable_lcd();


#if POKEMON_VERSION == RED
	auto title_species = SpeciesId::Charmander;
#elif POKEMON_VERSION == BLUE
	auto title_species = SpeciesId::Squirtle;
#else
#error Pokemon version not defined!
#endif
	//Note: This call invalidates most of the VRAM tile map! The PC tiles, the
	//Pokemon logo tiles, and the copyright stuff all get erased. All that
	//remains besides the mon is the version tiles and the special symbols of
	//the charmap. I have no idea why GameFreak left the previous load routine
	//calls, as they have basically no effect and just waste time and power.
	this->load_mon_sprite(title_species);

	this->copy_tilemap_to_vram((vBGMap0 + 0x0300) >> 8);
	this->parent->save_screen_tiles_to_buffer1();
	this->hram.hWY = 0x40;
	this->parent->load_screen_tiles_from_buffer2();
	this->copy_tilemap_to_vram(vBGMap0 >> 8);
	this->parent->gb_pal_normal();
	this->parent->OBP0 = 0xE4;

	this->bounce_logo();

	this->parent->load_screen_tiles_from_buffer1();
	this->parent->delay_frames(36);
	this->parent->play_sound(Sound::SFX_Intro_Whoosh);

	this->scroll_in_game_version();

	this->copy_tilemap_to_vram(vBGMap1 >> 8);
	this->parent->load_screen_tiles_from_buffer2();
	this->print_game_version();
	this->parent->delay3();
	this->parent->wait_for_sound_to_finish();
	this->parent->play_sound(this->wram.wNewSoundID = Sound::Music_TitleScreen);
	this->wram.wUnusedCC5B = 0;

	this->mon_scroll_loop();
	this->parent->play_cry(this->wram.wTitleMonSpecies);
	this->parent->wait_for_sound_to_finish();
	this->parent->gb_pal_white_out_with_delay3();
	this->parent->clear_sprites();
	this->hram.hWY = 0;
	this->hram.H_AUTOBGTRANSFERENABLED = 1;
	this->parent->clear_screen();
	this->copy_tilemap_to_vram(vBGMap0 >> 8);
	this->copy_tilemap_to_vram(vBGMap1 >> 8);
	this->parent->delay3();
	this->parent->load_gb_pal();

	return CppRed::check_for_data_clear_request(this->hram.hJoyHeld) ?
		TitleScreenResult::GoToClearSaveDialog :
		TitleScreenResult::GoToMainMenu;
}


void CppRedTitleScreen::copy_pokemon_logo_to_wram_tilemap(){
	unsigned tile = 0x80;
	auto dst = this->parent->get_tilemap_location(2, 1);
	for (unsigned row = 0; row < 6; row++){
		for (unsigned column = 0; column < 16; column++)
			dst[column] = tile++;
		dst += tilemap_width;
	}

	//Redundant assignment:
	//dst = this->get_tilemap_location(2, 7);

	tile = 0x31;
	for (unsigned column = 0; column < 16; column++)
		dst[column] = tile++;
}

void CppRedTitleScreen::copy_copyright_to_wram_tilemap(){
	//©'95.'96.'98 GAME FREAK inc.
	static const byte_t copyright_tiles[] = {
		0x41, 0x42, 0x43, 0x42, 0x44, 0x42, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
		0x4B, 0x4C, 0x4D, 0x4E
	};

	auto dst = this->parent->get_tilemap_location(2, 17);
	for (unsigned column = 0; column < 16; column++)
		dst[column] = copyright_tiles[column];

}

void CppRedTitleScreen::bounce_logo(){
	static const std::pair<int, int> logo_y_scrolls[] = {
		{ -4, 16 },
		{ 3,  4 },
		{ -3,  4 },
		{ 2,  2 },
		{ -2,  2 },
		{ 1,  2 },
		{ -1,  2 },
	};

	for (auto &pair : logo_y_scrolls){
		if (pair.first == -3)
			this->parent->play_sound(Sound::SFX_Intro_Crash);
		unsigned y = this->hram.hSCY;
		this->scroll_logo(pair, y);
		this->hram.hSCY = y;
	}
}

void CppRedTitleScreen::scroll_in_game_version(){
	this->print_game_version();
	this->hram.hWY = lcd_height;
	for (unsigned d = lcd_height; d; d = (d + 4) & 0xFF){
		this->scroll_game_version(d, 64);
		this->scroll_game_version(0, 80);
	}
}

void CppRedTitleScreen::mon_scroll_loop(){
	while (!this->parent->check_for_user_interruption(200)){
		this->scroll_in_mon();
		if (this->parent->check_for_user_interruption(1))
			break;
		this->animate_ball_if_starter_out();
		this->pick_new_mon();
	}
}

void CppRedTitleScreen::draw_player_character(){
	auto pc = decode_image_data(PlayerCharacterTitleGraphics);
	auto dst = &this->parent->get_display_controller().access_vram(vSprites);
	memcpy(dst, &pc[0], pc.size());
	this->parent->clear_sprites();
	this->wram.wPlayerCharacterOAMTile = 0;
	auto it = this->wram.wOAMBuffer.begin();
	unsigned coord_y = 96;
	for (int i = 7; i--;){
		unsigned coord_x = 90;
		for (int j = 5; j--;){
			auto sprite = *it;
			sprite.y_position = coord_y;
			sprite.x_position = coord_x;
			sprite.tile_number = this->wram.wPlayerCharacterOAMTile++;
			it++;
			coord_x = (coord_x + 8) & 0xFF;
		}
		coord_y = (coord_y + 8) & 0xFF;
	}
}

static const unsigned copyright_tiles_to_copy = 5;
static const unsigned logo_second_fragment_size = 0x100;

void CppRedTitleScreen::load_pokemon_logo(){
	auto image_data = decode_image_data(PokemonLogoGraphics);
	this->parent->copy_video_data(&image_data[0x0000], 0x600, vTitleLogo);
	assert(image_data.size() - 0x600 == logo_second_fragment_size);
	this->parent->copy_video_data(&image_data[0x0600], logo_second_fragment_size, vTitleLogo2);
}

void CppRedTitleScreen::load_copyright_graphics(){
	this->parent->copy_video_data(NintendoCopyrightLogoGraphics, copyright_tiles_to_copy, 0, vTitleLogo2 + logo_second_fragment_size);
}

void CppRedTitleScreen::load_gamefreak_logo(){
	this->parent->copy_video_data(GamefreakLogoGraphics, vTitleLogo2 + logo_second_fragment_size + copyright_tiles_to_copy * tile_byte_size);
}

void CppRedTitleScreen::load_version_graphics(){
#if POKEMON_VERSION == RED
	auto &image = Version_GFX_red;
	unsigned destination = vChars2 + 0x600;
#elif POKEMON_VERSION == BLUE
	auto &image = Version_GFX_blue;
	unsigned destination = vChars2 + 0x601;
#else
#error Pokemon version not defined!
#endif
	this->parent->copy_video_data(image, destination);
}

void CppRedTitleScreen::load_mon_sprite(SpeciesId species){
	this->wram.wTitleMonSpecies = species;
	auto destination = this->parent->get_tilemap_location(5, 10);
	this->parent->load_front_sprite(species, false, destination);
}

void CppRedTitleScreen::copy_tilemap_to_vram(unsigned destination_page){
	auto &destination = this->parent->hram.H_AUTOBGTRANSFERDEST;
	auto value = +destination;
	value &= ~0xFF;
	value |= destination_page & 0xFF;
	destination = value;
	this->parent->delay3();
}

void CppRedTitleScreen::print_game_version(){
	static const byte_t version_string_red[] = { 0x60, 0x61, 0x7F, 0x65, 0x66, 0x67, 0x68, 0x69 };
	static const byte_t version_string_blue[] = { 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68 };
#if POKEMON_VERSION == RED
	static auto &version_string = version_string_red;
#elif POKEMON_VERSION == BLUE
	static auto &version_string = version_string_blue;
#else
#error Pokemon version not defined!
#endif

	auto location = this->parent->get_tilemap_location(7, 8);
	for (auto c : version_string)
		*(location++) = c;
}

void CppRedTitleScreen::scroll_game_version(unsigned h, unsigned l){
	this->parent->wait_for_ly(l);
	this->parent->SCX = h;
	this->parent->wait_while_ly(h);
}

void CppRedTitleScreen::scroll_logo(const std::pair<int, int> &amount, unsigned &y_scroll){
	for (int i = amount.second; i--;){
		this->parent->delay_frame();
		y_scroll += amount.first;
	}
}

void CppRedTitleScreen::scroll_in_mon(){
	this->title_scroll(false);
	this->parent->hram.hWY = 0;
}

struct TitleScrollData{
	byte_t speed;
	byte_t duration;
};

static const TitleScrollData title_scroll_durations_in[] = {
	{ 10, 2 },
	{  9, 4 },
	{  8, 4 },
	{  6, 3 },
	{  5, 2 },
	{  3, 1 },
	{  1, 1 },
};
static const TitleScrollData title_scroll_durations_out[] = {
	{ 1, 2 },
	{ 2, 2 },
	{ 3, 2 },
	{ 4, 2 },
	{ 5, 2 },
	{ 6, 2 },
	{ 8, 3 },
	{ 9, 3 },
};
static const TitleScrollData title_scroll_waitball[] = {
	{0, 5},
	{0, 5},
};
static const byte_t title_ball_table[] = { 0x71, 0x6f, 0x6e, 0x6d, 0x6c, 0x6d, 0x6e, 0x6f, 0x71, 0x74 };

static const byte_t get_title_ball_y(CppRed &red, int &i){
	if (i < 0 || i >= array_length(title_ball_table))
		return 0;
	return red.wram.wOAMBuffer[10].y_position = title_ball_table[i++];
}

void CppRedTitleScreen::title_scroll2(const TitleScrollData *data, size_t data_length, unsigned d, int index){
	for (size_t i = 0; i < data_length; i++){
		auto current = data[i];
		for (unsigned j = 0; j < current.duration; j++){
			this->scroll_game_version(d, 0x48);
			this->scroll_game_version(0, 0x88);
			d += current.speed;
			get_title_ball_y(*this->parent, index);
		}
	}
}

void CppRedTitleScreen::title_scroll(bool out){
	const TitleScrollData *data;
	size_t data_length;
	unsigned d;
	if (!out){
		data = title_scroll_durations_in;
		data_length = array_length(title_scroll_durations_in);
		d = 0x88;
	}else{
		data = title_scroll_durations_out;
		data_length = array_length(title_scroll_durations_out);
		d = 0;
	}

	this->title_scroll2(data, data_length, d, -1);
}

void CppRedTitleScreen::animate_ball_if_starter_out(){
	auto species = this->parent->wram.wTitleMonSpecies.enum_value();
	bool found = false;
	for (auto starter : CppRed::starter_mons){
		if (species == starter){
			found = true;
			break;
		}
	}
	if (!found)
		return;

	this->title_scroll2(title_scroll_waitball, array_length(title_scroll_waitball), 0, -1);
}

static const SpeciesId title_screen_mons[] = {
#if POKEMON_VERSION == RED
	SpeciesId::Charmander,
	SpeciesId::Squirtle,
	SpeciesId::Bulbasaur,
	SpeciesId::Weedle,
	SpeciesId::NidoranMale,
	SpeciesId::Scyther,
	SpeciesId::Pikachu,
	SpeciesId::Clefairy,
	SpeciesId::Rhydon,
	SpeciesId::Abra,
	SpeciesId::Gastly,
	SpeciesId::Ditto,
	SpeciesId::Pidgeotto,
	SpeciesId::Onix,
	SpeciesId::Ponyta,
	SpeciesId::Magikarp,
#elif POKEMON_VERSION == BLUE
	SpeciesId::Squirtle,
	SpeciesId::Charmander,
	SpeciesId::Bulbasaur,
	SpeciesId::Mankey,
	SpeciesId::Hitmonlee,
	SpeciesId::Vulpix,
	SpeciesId::Chansey,
	SpeciesId::Aerodactyl,
	SpeciesId::Jolteon,
	SpeciesId::Snorlax,
	SpeciesId::Gloom,
	SpeciesId::Poliwag,
	SpeciesId::Doduo,
	SpeciesId::Porygon,
	SpeciesId::Gengar,
	SpeciesId::Raichu,
#else
#error Pokemon version not defined!
#endif
};

void CppRedTitleScreen::pick_new_mon(){
	this->copy_tilemap_to_vram(vBGMap0 >> 8);

	auto old_species = this->parent->wram.wTitleMonSpecies.enum_value();
	
	SpeciesId new_species = SpeciesId::None;
	do
		new_species = title_screen_mons[this->parent->random() % array_length(title_screen_mons)];
	while (new_species == old_species);

	this->load_mon_sprite(new_species);

	this->parent->hram.hWY = 0x90;
	this->title_scroll(true);
}
