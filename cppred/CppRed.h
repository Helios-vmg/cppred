#pragma once
#include "DisplayController.h"
#include "UserInputController.h"
#include "SystemClock.h"
#include "SoundController.h"
#include "CommonTypes.h"
#include "CppRedConstants.h"
#include "CppRedRam.h"
#include "CppRedAudio.h"
#include "CppRedText.h"
#include "CppRedMainMenu.h"
#include "CppRedMap.h"
#include "ExternalRamBuffer.h"
#include "threads.h"
#include "utility.h"
#include <memory>
#include <functional>
#include <atomic>

enum class TitleScreenResult;
class HostSystem;

class CppRed{
	HostSystem *host;
	DisplayController display_controller;
	UserInputController input_controller;
	SoundController sound_controller;
	SystemClock clock;
	CppRedAudio audio;
	//std::unique_ptr<byte_t[]> emulated_memory;
	std::function<void()> predef_functions[(int)Predef::COUNT];
	std::atomic<bool> continue_running;
	std::atomic<bool> paused;
	bool registered = false;
	Event periodic_notification;
	std::unique_ptr<std::thread> interpreter_thread;
	double accumulated_time = -1;
	Maybe<posix_time_t> start_time;
	double real_time_multiplier;
	double speed_multiplier = 1;
	std::uint64_t realtime_counter_frequency = 0;
	std::uint64_t current_timer_start;
	//ExternalRamBuffer ram_to_save;
	std::uint64_t time_running = 0;
	std::uint64_t time_waiting = 0;
	bool speed_changed = false;
	byte_t interrupt_flag = 0;
	byte_t interrupt_enable_flag = 0;

	void interpreter_thread_function();
	void init();
	//void run_until_next_frame();
	//void sync_with_real_time();
	double get_real_time();
	//void execute_pause();
	void main();
	void start_new_game();
	void start_loaded_game();
	void special_enter_map(MapId);
	MapId special_warp_in();
	void update_player_sprite();
	void update_non_player_sprite(const SpriteStateData2 &);
	void detect_sprite_collision();
	void do_scripted_npc_movement();
	void update_npc_sprite(const SpriteStateData2 &);
	void initialize_scripted_npc_movement();
	void anim_scripted_npc_movement();
public:

	WRam wram;
	HRam hram;
	CppRedText text;

	//Simulated hardware registers:
#define DECLARE_HARDWARE_REGISTER(name) ArbitraryIntegerWrapper<byte_t, 1> name
	//Serial Byte
	DECLARE_HARDWARE_REGISTER(SB);
	//Serial Control
	DECLARE_HARDWARE_REGISTER(SC);
	//Timer Modulo
	DECLARE_HARDWARE_REGISTER(TMA);
	//Timer Control
	DECLARE_HARDWARE_REGISTER(TAC);
	//Background Palette
	DECLARE_HARDWARE_REGISTER(BGP);
	//Object Palette 0
	DECLARE_HARDWARE_REGISTER(OBP0);
	//Object Palette 1
	DECLARE_HARDWARE_REGISTER(OBP1);
	//LCD Control
	DECLARE_HARDWARE_REGISTER(LCDC);
	//LCD STATus
	DECLARE_HARDWARE_REGISTER(STAT);
	//Interrupt Flag
	DECLARE_HARDWARE_REGISTER(IF);
	//Interrupt Enable Flag
	DECLARE_HARDWARE_REGISTER(IE);
	DECLARE_HARDWARE_REGISTER(InterruptMasterFlag);
	DECLARE_HARDWARE_REGISTER(SCX);
	DECLARE_HARDWARE_REGISTER(SCY);
	DECLARE_HARDWARE_REGISTER(WX);
	DECLARE_HARDWARE_REGISTER(WY);
	DECLARE_HARDWARE_REGISTER(LY);
	DECLARE_HARDWARE_REGISTER(LYC);

	CppRed(HostSystem &host);
	void run();
	void set_bg_scroll(int x = -1, int y = -1);
	void set_window_position(int x = -1, int y = -1);
	void nonemulation_init();
	void enable_lcd();
	//Disables the LCD causing a screen clear. Blocks until vsync.
	void disable_lcd();
	void clear_vram();
	void clear_wram();
	void clear_hram();
	void clear_sprites();
	void clear_bg_map(unsigned page);
	void stop_all_sounds();
	void execute_predef(Predef);
	void gb_pal_normal();
	void gb_pal_whiteout();
	void gb_pal_white_out_with_delay3(){
		this->gb_pal_whiteout();
	}
	void set_default_names_before_titlescreen();
	TitleScreenResult display_titlescreen();
	void clear_screen();
	void load_font_tile_patterns();
	void load_textbox_tile_patterns();
	void load_copyright_graphics();
	void load_gamefreak_logo();
	void load_pokemon_logo();
	void load_version_graphics();
	void clear_both_bg_maps();
	typedef decltype(WRam::wTileMap)::iterator tilemap_it;
	//Note: In the disassembly, the analog to this function is the macro coord.
	tilemap_it get_tilemap_location(unsigned x, unsigned y);
	void save_screen_tiles_to_buffer2();
	void load_screen_tiles_from_buffer1();
	void load_screen_tiles_from_buffer2();
	void load_screen_tiles_from_buffer2_disable_bg_transfer();
	void save_screen_tiles_to_buffer1();
	void run_palette_command(PaletteCommand cmd);
	void play_sound(Sound);
	void delay_frames(unsigned count);
	void delay3();
	void wait_for_sound_to_finish();
	void play_cry(SpeciesId);
	Sound get_cry_data(SpeciesId);
	void load_gb_pal();
	void display_clear_save_dialog();
	MainMenuResult display_main_menu();
	void run_default_palette_command(){
		this->run_palette_command(PaletteCommand::Default);
	}
	void print_text(const CppRedText::Region &);
	void display_textbox_id(const tilemap_it &location, unsigned unk0, unsigned unk1);
	void clear_save();
	DisplayController &get_display_controller(){
		return this->display_controller;
	}
	void prepare_menu(){
		this->clear_screen();
		this->run_default_palette_command();
		this->load_textbox_tile_patterns();
		this->load_font_tile_patterns();
	}
	void update_sprites();
	byte_t handle_menu_input();
	void joypad();

	static const unsigned vblank_flag_bit = 0;
	static const unsigned lcd_stat_flag_bit = 1;
	static const unsigned timer_flag_bit = 2;
	static const unsigned serial_flag_bit = 3;
	static const unsigned joypad_flag_bit = 4;
	static const unsigned all_interrupts_mask = (1 << 5) - 1;

	static const unsigned vblank_mask   = 1 << vblank_flag_bit;
	static const unsigned lcd_stat_mask = 1 << lcd_stat_flag_bit;
	static const unsigned timer_mask    = 1 << timer_flag_bit;
	static const unsigned serial_mask   = 1 << serial_flag_bit;
	static const unsigned joypad_mask   = 1 << joypad_flag_bit;

	static const unsigned vblank_interrupt_handler_address    = 0x0040 + 8 * vblank_flag_bit;
	static const unsigned lcd_stat_interrupt_handler_address  = 0x0040 + 8 * lcd_stat_flag_bit;
	static const unsigned timer_interrupt_handler_address     = 0x0040 + 8 * timer_flag_bit;
	static const unsigned serial_interrupt_handler_address    = 0x0040 + 8 * serial_flag_bit;
	static const unsigned joypad_interrupt_handler_address    = 0x0040 + 8 * joypad_flag_bit;
};

void cppred_main();
