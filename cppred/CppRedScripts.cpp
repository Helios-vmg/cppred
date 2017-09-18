#include "CppRedScripts.h"
#include "CppRed.h"
#include "../CodeGeneration/output/gfx.h"
#include "CppRedText.h"

namespace CppRedScripts{

//------------------------------------------------------------------------------
//                                 OAK SPEECH
//------------------------------------------------------------------------------

void fade_in_intro_pic(CppRed &red);
void move_pic_left(CppRed &red);
void choose_player_name(CppRed &red);
void choose_rival_name(CppRed &red);
void slide_pic_right(CppRed &red);
void slide_pic_left(CppRed &red);
int display_intro_name_textbox(CppRed &red, const char * const *names);

void oak_speech(CppRed &red){
	auto &text = red.text;
	red.play_sound(Sound::Stop);
	red.play_sound(Sound::Music_Routes2);
	red.clear_screen();
	red.load_textbox_tile_patterns();
	red.initialize_player_data();
	red.call_predef(Predef::InitPlayerData2);
	red.add_item_to_inventory(0, ItemId::Potion, 1);
	red.wram.wMainData.wDestinationMap = red.wram.wDefaultMap;
	red.special_warp_in();
	red.hram.hTilesetType = 0;
	
	//Skip choosing names?
	if (!red.wram.wMainData.wd732.get_unknown()){
		red.display_picture_centered_or_upper_right(ProfOakPic, Placing::Centered);
		fade_in_intro_pic(red);
		red.print_text(text.OakSpeechText1);
		red.gb_fadeout_to_white();
		red.clear_screen();

		red.load_front_sprite(SpeciesId::Nidorino, true, red.get_tilemap_location(6, 4));
		move_pic_left(red);
		red.print_text(text.OakSpeechText2A);
		red.gb_fadeout_to_white();
		red.clear_screen();

		red.display_picture_centered_or_upper_right(RedPicFront, Placing::Centered);
		move_pic_left(red);
		red.print_text(text.IntroducePlayerText);
		choose_player_name(red);
		red.gb_fadeout_to_white();
		red.clear_screen();

		red.display_picture_centered_or_upper_right(Rival1Pic, Placing::Centered);
		fade_in_intro_pic(red);
		red.print_text(text.IntroduceRivalText);
		choose_rival_name(red);
	}

	red.gb_fadeout_to_white();
	red.clear_screen();
	red.display_picture_centered_or_upper_right(RedPicFront, Placing::Centered);
	red.gb_fadein_from_white();

	red.print_text(text.OakSpeechText3);
	red.play_sound(Sound::SFX_Shrink_1);
	red.delay_frames(4);
	red.copy_video_data(RedSprite, 12, 0, vSprites);
	red.display_picture_centered_or_upper_right(ShrinkPic1, Placing::Centered);
	red.delay_frames(4);
	red.display_picture_centered_or_upper_right(ShrinkPic2, Placing::Centered);
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

void move_pic_left(CppRed &red){
	red.WX = 119;
	red.delay_frame();
	red.BGP = bits_from_u32<0x11100100>::value;
	while (true){
		red.delay_frame();
		auto x = red.WX - 8;
		if (x == 0xFF)
			return;
		red.WX = x;
	}
}

void fade_in_intro_pic(CppRed &red){
	static const byte_t palettes[] = {
		bits_from_u32<0x01010100>::value,
		bits_from_u32<0x10101000>::value,
		bits_from_u32<0x11111100>::value,
		bits_from_u32<0x11111000>::value,
		bits_from_u32<0x11110100>::value,
		bits_from_u32<0x11100100>::value,
	};

	for (auto palette : palettes){
		red.BGP = palette;
		red.delay_frames(10);
	}
}

static const char * const default_names_red[] = {
	"NEW NAME",
	"RED",
	"ASH",
	"JACK",
	nullptr,
};

static const char * const default_names_blue[] = {
	"NEW NAME",
	"BLUE",
	"GARY",
	"JOHN",
	nullptr,
};

void choose_character_name(CppRed &red, bool is_rival){
#if POKEMON_VERSION == RED
	static decltype(default_names_red) * const default_names[] = {
		&default_names_red,
		&default_names_blue,
	};
#elif POKEMON_VERSION == BLUE
	static decltype(default_names_red) * const default_names[] = {
		&default_names_blue,
		&default_names_red,
	};
#else
#error Pokemon version not defined!
#endif

	auto &name_array = *default_names[(int)is_rival];
	auto &name_dst = !is_rival ? red.wram.wPlayerName : red.wram.wMainData.wRivalName;
	const auto name_screen_type = !is_rival ? NamingScreenType::PlayerName : NamingScreenType::RivalName;
	auto &character_image = !is_rival ? RedPicFront : Rival1Pic;
	auto &ok_text = !is_rival ? red.text.YourNameIsText : red.text.HisNameIsText;

	slide_pic_right(red);

	auto selection = display_intro_name_textbox(red, name_array);
	assert(selection >= 0 && selection < array_length(name_array) - 1);
	if (selection){
		name_dst = name_array[selection];
		slide_pic_left(red);
	}else{
		//Custom name.
		std::string name;
		do
			name = red.display_naming_screen(name_screen_type);
		while (!name.size());
		name_dst = name;
		red.clear_screen();
		red.delay3();
		red.display_picture_centered_or_upper_right(character_image, Placing::Centered);
	}
	red.text.print_text(ok_text);
}

void choose_player_name(CppRed &red){
	choose_character_name(red, false);
}

void choose_rival_name(CppRed &red){
	choose_character_name(red, true);
}

//------------------------------------------------------------------------------

}
