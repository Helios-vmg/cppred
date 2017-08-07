#include "CppRedScripts.h"
#include "CppRed.h"
#include "../CodeGeneration/output/gfx.h"
#include "CppRedText.h"

namespace CppRedScripts{

enum class Placing{
	Centered,
	TopLeft,
};

//------------------------------------------------------------------------------
//                                 OAK SPEECH
//------------------------------------------------------------------------------

void intro_display_picture_centered_or_upper_right(CppRed &red, const BaseStaticImage &image, Placing placing);
void fade_in_intro_pic(CppRed &red);
void move_pic_left(CppRed &red);
void choose_player_name(CppRed &red);
void choose_rival_name(CppRed &red);

void oak_speech(CppRed &red){
	auto &text = red.text;
	red.play_sound(Sound::Stop);
	red.play_sound(Sound::Music_Routes2);
	red.clear_screen();
	red.load_textbox_tile_patterns();
	red.initialize_player_data();
	red.call_predef(Predef::InitPlayerData2);
	red.add_item_to_inventory(0, ItemId::Potion, 1);
	red.wram.wDestinationMap = red.wram.wDefaultMap;
	red.special_warp_in();
	red.hram.hTilesetType = 0;
	
	//Skip choosing names?
	if (!red.wram.wd732.get_unknown()){
		intro_display_picture_centered_or_upper_right(red, ProfOakPic, Placing::Centered);
		fade_in_intro_pic(red);
		red.print_text(text.OakSpeechText1);
		red.gb_fadeout_to_white();
		red.clear_screen();

		red.load_front_sprite(SpeciesId::Nidorino, true, red.get_tilemap_location(6, 4));
		move_pic_left(red);
		red.print_text(text.OakSpeechText2A);
		red.gb_fadeout_to_white();
		red.clear_screen();

		intro_display_picture_centered_or_upper_right(red, RedPicFront, Placing::Centered);
		red.move_pic_left();
		red.print_text(text.IntroducePlayerText);
		choose_player_name(red);
		red.gb_fadeout_to_white();
		red.clear_screen();

		intro_display_picture_centered_or_upper_right(red, Rival1Pic, Placing::Centered);
		fade_in_intro_pic(red);
		red.print_text(text.IntroduceRivalText);
		choose_rival_name(red);
	}

	red.gb_fadeout_to_white();
	red.clear_screen();
	intro_display_picture_centered_or_upper_right(red, RedPicFront, Placing::Centered);
	red.gb_fadein_from_white();

	red.print_text(text.OakSpeechText3);
	red.play_sound(Sound::SFX_Shrink_1);
	red.delay_frames(4);
	red.copy_video_data(12, RedSprite, 0, vSprites);
	intro_display_picture_centered_or_upper_right(red, ShrinkPic1, Placing::Centered);
	red.delay_frames(4);
	intro_display_picture_centered_or_upper_right(red, ShrinkPic2, Placing::Centered);
	red.reset_player_sprite_data();
	red.wram.wAudioFadeOutControl = 10;
	red.wram.wNewSoundID = Sound::Stop;
	red.play_sound(Sound::Music_PalletTown);
	red.delay_frames(20);
	red.clear_screen_area(7, 7, red.get_tilemap_location(6, 5));
	red.load_textbox_tile_patterns();
	red.wram.wUpdateSpritesEnabled = 1;
	red.delay_frames(50);
	red.gb_fadeout_to_white();
	red.clear_screen();
}

//------------------------------------------------------------------------------

}
