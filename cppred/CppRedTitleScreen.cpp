#include "CppRedTitleScreen.h"
#include "CppRed.h"
#include "../CodeGeneration/output/gfx.h"

CppRedTitleScreen::CppRedTitleScreen(CppRed &parent):
	parent(parent),
	wram(parent.wram),
	hram(parent.hram){
}

TitleScreenResult CppRedTitleScreen::display(){
	this->parent.gb_pal_whiteout();
	this->hram.H_AUTOBGTRANSFERENABLED = 1;
	this->hram.hTilesetType = 0;
	this->hram.hSCX = 0;
	this->hram.hSCY = 0x40;
	this->hram.hWY = 0x90;
	this->parent.clear_screen();
	this->parent.disable_lcd();
	//Note: this call makes little sense here, as most of the characters get
	//overwritten by the Pokemon logo.
	this->parent.load_font_tile_patterns();
	this->load_copyright_graphics();
	this->load_pokemon_logo();
	this->load_version_graphics();
	this->parent.clear_both_bg_maps();
	this->copy_pokemon_logo_to_wram_tilemap();
	this->draw_player_character();

	//Put a pokeball in the player's hand.
	this->wram.wOAMBuffer[0x28] = 0x74;

	this->copy_copyright_to_wram_tilemap();
	this->parent.save_screen_tiles_to_buffer2();
	this->parent.load_screen_tiles_from_buffer2();
	this->parent.enable_lcd();

#if POKEMON_VERSION == RED
	this->wram.wTitleMonSpecies = SpeciesId::Charmander;
#elif POKEMON_VERSION == BLUE
	this->wram.wTitleMonSpecies = SpeciesId::Squirtle;
#else
#error Pokemon version not defined!
#endif
	//Note: This call invalidates most of the VRAM tile map! The PC tiles, the
	//Pokemon logo tiles, and the copyright stuff all get erased. All that
	//remains besides the mon is the version tiles and the special symbols of
	//the charmap. I have no idea why GameFreak left the previous load routine
	//calls, as they have basically no effect and just waste time and power.
	this->load_mon_sprite();

	this->copy_tilemap_to_vram((bg_map0 + 0x0300) >> 8);
	this->parent.save_screen_tiles_to_buffer1();
	this->hram.hWY = 0x40;
	this->parent.load_screen_tiles_from_buffer2();
	this->copy_tilemap_to_vram(bg_map0 >> 8);
	this->parent.run_palette_command(PaletteCommand::SetPaletteTitleScreen);
	this->parent.gb_pal_normal();
	this->parent.OBP0 = 0xE4;

	this->bounce_logo();

	this->parent.load_screen_tiles_from_buffer1();
	this->parent.delay_frames(36);
	this->parent.play_sound(Sound::SFX_Intro_Whoosh);

	this->scroll_in_game_version();

	this->copy_tilemap_to_vram(bg_map1 >> 8);
	this->parent.load_screen_tiles_from_buffer2();
	this->print_game_version();
	this->parent.delay3();
	this->parent.wait_for_sound_to_finish();
	this->parent.play_sound(this->wram.wNewSoundID = Sound::Music_TitleScreen);
	this->wram.wUnusedCC5B = 0;

	this->mon_scroll_loop();
	this->parent.play_cry(this->wram.wTitleMonSpecies);
	this->parent.wait_for_sound_to_finish();
	this->parent.gb_pal_white_out_with_delay3();
	this->parent.clear_sprites();
	this->hram.hWY = 0;
	this->hram.H_AUTOBGTRANSFERENABLED = 1;
	this->parent.clear_screen();
	this->copy_tilemap_to_vram(bg_map0 >> 8);
	this->copy_tilemap_to_vram(bg_map1 >> 8);
	this->parent.delay3();
	this->parent.load_gb_pal();

	const auto mask = input_up | input_select | input_b;
	return check_flag(this->hram.hJoyHeld, mask) ?
		TitleScreenResult::GoToClearSaveDialog :
		TitleScreenResult::GoToMainMenu;
}


void CppRedTitleScreen::copy_pokemon_logo_to_wram_tilemap(){
	unsigned tile = 0x80;
	auto dst = this->parent.get_tilemap_location(2, 1);
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

	auto dst = this->parent.get_tilemap_location(2, 17);
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
			this->parent.play_sound(Sound::SFX_Intro_Crash);
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
	while (!this->check_for_user_interruption(200)){
		this->scroll_in_mon();
		if (this->check_for_user_interruption(1))
			break;
		this->animate_ball_if_starter_out();
		this->pick_new_mon();
	}
}

void CppRedTitleScreen::draw_player_character(){
	auto pc = decode_image_data(PlayerCharacterTitleGraphics);
	auto dst = &this->parent.get_display_controller().access_vram(vSprites);
	memcpy(dst, &pc[0], pc.size());
	this->parent.clear_sprites();
	this->wram.wPlayerCharacterOAMTile = 0;
	auto it = this->wram.wOAMBuffer.begin();
	unsigned coord_y = 96;
	for (int i = 7; i--;){
		unsigned coord_x = 90;
		for (int j = 5; j--;){
			//y position
			*(it++) = coord_y;
			//x position
			*(it++) = coord_x;
			//tile
			*(it++) = this->wram.wPlayerCharacterOAMTile++;
			//skip attributes
			it++;

			coord_x = (coord_x + 8) & 0xFF;
		}
		coord_y = (coord_y + 8) & 0xFF;
	}
}

void CppRedTitleScreen::load_copyright_graphics(){
	//TODO:
	/*
	* ld hl, NintendoCopyrightLogoGraphics
	* ld de, vTitleLogo2 + $100
	* ld bc, $50
	* ld a, BANK(NintendoCopyrightLogoGraphics)
	* call FarCopyData2
	*/
}

void CppRedTitleScreen::load_gamefreak_logo(){
	//TODO:
	/*
	* ld hl, GamefreakLogoGraphics
	* ld de, vTitleLogo2 + $100 + $50
	* ld bc, $90
	* ld a, BANK(GamefreakLogoGraphics)
	* call FarCopyData2
	*/
}

void CppRedTitleScreen::load_pokemon_logo(){
	//TODO:
	/*
	* ld hl, PokemonLogoGraphics
	* ld de, vTitleLogo
	* ld bc, $600
	* ld a, BANK(PokemonLogoGraphics)
	* call FarCopyData2          ; first chunk
	* ld hl, PokemonLogoGraphics+$600
	* ld de, vTitleLogo2
	* ld bc, $100
	* ld a, BANK(PokemonLogoGraphics)
	* call FarCopyData2          ; second chunk
	*/
}

void CppRedTitleScreen::load_version_graphics(){
	//TODO:
	/*
	* ld hl, Version_GFX
	* ld de,vChars2 + $600 - (Version_GFXEnd - Version_GFX - $50)
	* ld bc, Version_GFXEnd - Version_GFX
	* ld a, BANK(Version_GFX)
	* call FarCopyDataDouble
	*/
}
