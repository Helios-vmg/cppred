#include "CppRedIntro.h"
#include "CppRed.h"
#include "StaticImage.h"
#include "../CodeGeneration/output/pokemon_declarations.h"

namespace CppRedIntro{

typedef decltype(WRam::wTileMap)::iterator tilemap_it;
static const unsigned default_intro_bar_thickness = 4;
static const unsigned small_star_copies = 24;

static void load_shooting_star_graphics(CppRed &red){
	static const SimpleSpriteObject game_freak_logo_oam_data[] = {
		{ 0x48, 0x50, 0x8D, 0x00 },
		{ 0x48, 0x58, 0x8E, 0x00 },
		{ 0x50, 0x50, 0x8F, 0x00 },
		{ 0x50, 0x58, 0x90, 0x00 },
		{ 0x58, 0x50, 0x91, 0x00 },
		{ 0x58, 0x58, 0x92, 0x00 },
		{ 0x60, 0x30, 0x80, 0x00 },
		{ 0x60, 0x38, 0x81, 0x00 },
		{ 0x60, 0x40, 0x82, 0x00 },
		{ 0x60, 0x48, 0x83, 0x00 },
		{ 0x60, 0x50, 0x93, 0x00 },
		{ 0x60, 0x58, 0x84, 0x00 },
		{ 0x60, 0x60, 0x85, 0x00 },
		{ 0x60, 0x68, 0x83, 0x00 },
		{ 0x60, 0x70, 0x81, 0x00 },
		{ 0x60, 0x78, 0x86, 0x00 },
	};
	static const SimpleSpriteObject game_freak_shooting_star_oam_data[] = {
		{ 0x00, 0xA0, 0xA0, 0x10 },
		{ 0x00, 0xA8, 0xA0, 0x30 },
		{ 0x08, 0xA0, 0xA1, 0x10 },
		{ 0x08, 0xA8, 0xA1, 0x30 },
	};

	red.OBP0 = bits_from_u32<0x11111001>::value;
	red.OBP1 = bits_from_u32<0x10100100>::value;
	{
		auto data = decode_image_data(AnimationTileset2);
		red.copy_video_data(&data[tile_byte_size * 3], tile_byte_size, vChars1 + 0x200);
		red.copy_video_data(&data[tile_byte_size * 19], tile_byte_size, vChars1 + 0x200 + tile_byte_size);
	}
	red.copy_video_data(FallingStar, vChars1 + 0x200 + tile_byte_size * 2);

	std::copy(
		game_freak_logo_oam_data,
		game_freak_logo_oam_data + array_length(game_freak_logo_oam_data),
		red.wram.wOAMBuffer.begin() + small_star_copies
	);
	std::copy(
		game_freak_shooting_star_oam_data,
		game_freak_shooting_star_oam_data + array_length(game_freak_shooting_star_oam_data),
		red.wram.wOAMBuffer.begin()
	);
}

static bool big_star_animation(CppRed &red){
	auto star_sprites = red.wram.wOAMBuffer;

	do{
		for (int i = 0; i < 4; i++){
			star_sprites[i].y_position += 4;
			star_sprites[i].x_position -= 4;
		}
		if (red.check_for_user_interruption(1))
			return true;
	}while (star_sprites[0].y_position != lcd_width);
	
	return false;
}

static bool flash_game_freak_logo(CppRed &red){
	for (int i = 0; i < 3; i++){
		//Rotate palette.
		auto palette = +red.OBP0;
		red.OBP0 = (palette >> 2) | (palette << 6);
		if (red.check_for_user_interruption(10))
			return true;
	}
	return false;
}

static bool move_down_small_stars(CppRed &red, unsigned star_count){
	for (int i = 0; i < 8; i++){
		auto sprite = red.wram.wOAMBuffer.begin() + (small_star_copies - star_count);
		for (unsigned j = 0; j < star_count; j++)
			--sprite[j].y_position;

		red.OBP1 ^= bits_from_u32<0x10100000>::value;

		if (red.check_for_user_interruption(3))
			return true;
	}
	return false;
}

static bool animate_small_stars(CppRed &red){
	typedef std::pair<byte_t, byte_t> P;
	//These are coordinates in YX order.
	static const P wave1[] = {
		{ 0x68, 0x30 },
		{ 0x68, 0x40 },
		{ 0x68, 0x58 },
		{ 0x68, 0x78 },
		{ 0xFF, 0xFF },
	};
	static const P wave2[] = {
		{ 0x68, 0x38 },
		{ 0x68, 0x48 },
		{ 0x68, 0x60 },
		{ 0x68, 0x70 },
		{ 0xFF, 0xFF },
	};
	static const P wave3[] = {
		{ 0x68, 0x34 },
		{ 0x68, 0x4C },
		{ 0x68, 0x54 },
		{ 0x68, 0x64 },
		{ 0xFF, 0xFF },
	};
	static const P wave4[] = {
		{ 0x68, 0x3C },
		{ 0x68, 0x5C },
		{ 0x68, 0x6C },
		{ 0x68, 0x74 },
		{ 0xFF, 0xFF },
	};
	static const P wave5[] = {
		{ 0xFF, 0xFF },
	};
	static const P * const waves[] = {
		wave1,
		wave2,
		wave3,
		wave4,
		wave5,
		wave5,
	};

	unsigned star_count = 0;
	for (int i = 0; i < 6; i++){
		auto wave_pointer = waves[i];
		auto p2 = red.wram.wOAMBuffer.begin() + (small_star_copies - 4);
		bool skip = false;
		for (int j = 0; j < 4; j++){
			if (wave_pointer[i].first == 0xFF){
				skip = true;
				break;
			}
			(*p2).y_position = wave_pointer[i].first;
			(*p2).x_position = wave_pointer[i].second;
		}
		if (!skip && star_count < small_star_copies)
			star_count += 6;
		bool interrupted = move_down_small_stars(red, star_count);
		for (int j = 0; j < small_star_copies - 4; j++)
			red.wram.wOAMBuffer[j].assign(red.wram.wOAMBuffer[j + 4]);
		if (interrupted)
			return true;
	}
	return false;
}

//Returns true if the user interrupted the animation.
static bool gf_shooting_star_scene(CppRed &red){
	static const SimpleSpriteObject small_stars_oam_data = { 0x00, 0x00, 0xA2, 0x90 };

	load_shooting_star_graphics(red);
	red.play_sound(Sound::SFX_Shooting_Star);

	if (big_star_animation(red))
		return true;

	//Hide big star sprites.
	for (int i = 0; i < 4; i++)
		red.wram.wOAMBuffer[i].y_position = lcd_width;

	if (flash_game_freak_logo(red))
		return true;

	std::fill(
		red.wram.wOAMBuffer.begin(),
		red.wram.wOAMBuffer.begin() + small_star_copies,
		small_stars_oam_data
	);

	if (animate_small_stars(red))
		return true;

	return false;
}

static void clear_screen(CppRed &red){
	auto p = &red.get_display_controller().access_vram(vBGMap1);
	memset(p, 0, tilemap_height * vram_tilemap_width);
}

template <typename T>
static void place_black_tiles(CppRed &red, T dst, size_t n){
	while (n--)
		*(dst++) = 1;
}

template <unsigned N>
static void draw_black_bars(CppRed &red){
	static_assert(N * 2 < tilemap_height, "N * 2 must be less than tilemap_height!");

	clear_screen(red);
	auto it = red.get_tilemap_location(0, 0);
	place_black_tiles(red, it, tilemap_width * N);
	it = red.get_tilemap_location(0, tilemap_height - N);
	place_black_tiles(red, it, tilemap_width * N);
	auto p = &red.get_display_controller().access_vram(vBGMap1);
	place_black_tiles(red, p, vram_tilemap_width * N);
	place_black_tiles(red, p + vram_tilemap_width * (tilemap_height - N), vram_tilemap_width * N);
}

#if POKEMON_VERSION == RED
static const auto &FightIntroFrontMon1 = FightIntroFrontMon1_red;
static const auto &FightIntroFrontMon2 = FightIntroFrontMon2_red;
static const auto &FightIntroFrontMon3 = FightIntroFrontMon3_red;
#elif POKEMON_VERSION == BLUE
static const auto &FightIntroFrontMon1 = FightIntroFrontMon1_blue;
static const auto &FightIntroFrontMon2 = FightIntroFrontMon2_blue;
static const auto &FightIntroFrontMon3 = FightIntroFrontMon3_blue;
#else
#error Pokemon version not defined!
#endif

static void load_intro_graphics(CppRed &red){
	unsigned offset = 0;
	std::cout << "1\n";
	auto data = decode_image_data(FightIntroBackMon);
	std::cout << "2\n";
	red.copy_video_data(&data[0], data.size(), vChars2 + offset);
	std::cout << "3\n";
	offset += (unsigned)data.size();
	std::cout << "4\n";
	data = decode_image_data(GameFreakIntro);
	std::cout << "5\n";
	red.copy_video_data(&data[0], data.size(), vChars2 + offset);
	std::cout << "6\n";
	red.copy_video_data(&data[0], data.size(), vChars1);
	std::cout << "7\n";
	red.copy_video_data(FightIntroFrontMon1, vChars0);
	std::cout << "8\n";
}

template <unsigned N>
static void clear_middle_of_screen(CppRed &red){
	static_assert(N * 2 < tilemap_height, "N * 2 must be less than tilemap_height!");

	auto it = red.get_tilemap_location(0, N);
	std::fill(it, it + tilemap_width * (tilemap_height - N * 2), 0);
}

static void play_shooting_star(CppRed &red){
	std::cout << "play_shooting_star()\n";
	red.load_copyright_and_textbox_tiles();
	red.BGP = bits_from_u32<0x11100100>::value;
	//Hold copyright screen for three seconds.
	std::cout << "Holding.\n";
	red.delay_frames(180);
	std::cout << "Clear screen.\n";
	red.clear_screen();
	red.disable_lcd();
	red.wram.wCurOpponent = 0;
	std::cout << "Drawing bars.\n";
	draw_black_bars<default_intro_bar_thickness>(red);
	std::cout << "Loading graphics.\n";
	load_intro_graphics(red);
	std::cout << "Graphics loaded.\n";
	red.enable_lcd();
	red.LCDC &= 0xFF ^ DisplayController::lcdc_window_enable_mask;
	red.LCDC |= DisplayController::lcdc_bg_map_select_mask;
	red.delay_frames(64);
	std::cout << "Shooting star scene.\n";
	if (!gf_shooting_star_scene(red))
		red.delay_frames(40);
	std::cout << "Shooting star scene done.\n";
	red.play_sound(Sound::Music_IntroBattle);
	clear_middle_of_screen<default_intro_bar_thickness>(red);
	red.clear_sprites();
	red.delay3();
	std::cout << "Leaving play_shooting_star()\n";
}

static void play_intro_scene(CppRed &red){
	//TODO
}

void play(CppRed &red){
	red.hram.hJoyHeld.clear();
	red.hram.H_AUTOBGTRANSFERENABLED = 1;
	play_shooting_star(red);
	play_intro_scene(red);
	red.gb_fadeout_to_white();
	red.hram.hSCX = 0;
	red.hram.H_AUTOBGTRANSFERENABLED = 0;
	red.clear_sprites();
	red.delay_frame();
}

}
