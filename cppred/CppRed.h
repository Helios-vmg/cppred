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
#include "CppRedSRam.h"
#include "ExternalRamBuffer.h"
#include "CppRedMessageBox.h"
#include "threads.h"
#include "utility.h"
#include <memory>
#include <functional>
#include <atomic>

enum class TitleScreenResult;
class HostSystem;

class CppRed{
public:
	typedef decltype(WRam::wTileMap)::iterator tilemap_it;
	typedef std::array<byte_t, 0x8000> sram_t;
private:
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
	xorshift128_state random_state;
	std::vector<std::function<void()>> predefs;
	CppRedMessageBox message_box;

	void nonemulation_init();
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
	void enter_map(MapId);
	void update_player_sprite();
	void update_non_player_sprite(const SpriteStateData2 &);
	void detect_sprite_collision();
	void do_scripted_npc_movement();
	void update_npc_sprite(const SpriteStateData2 &);
	void initialize_scripted_npc_movement();
	void anim_scripted_npc_movement();
	void advance_scripted_npc_anim_frame_counter();
	void initialize_sprite_status();
	void initialize_sprite_screen_position();
	//Returns false if invisible
	bool check_sprite_availability();
	void make_npc_face_player(SpriteStateData1 &);
	void not_yet_moving(SpriteStateData1 &);
	void update_sprite_movement_delay();
	void update_sprite_in_walking_animation();
	tilemap_it get_tile_sprite_stands_on();
	std::uint32_t random();
	void change_facing_direction();
	bool try_walking(tilemap_it, int deltax, int deltay, DirectionBitmap movement_direction, SpriteFacingDirection facing_sprite_direction);
	bool can_walk_onto_tile(unsigned tile_id, DirectionBitmap direction, int deltax, int deltay);
	bool can_walk_onto_tile_helper(unsigned tile_id, DirectionBitmap direction, int deltax, int deltay);
	void update_sprite_image(SpriteStateData1 &);
	bool is_object_hidden();
	template <typename T>
	bool flag_action(FlagAction action, T &bitmap, unsigned bit){
		assert(bit < 0x100);
		auto index = bit / 8;
		bit &= 7;
		switch (action){
			case FlagAction::Reset:
				bitmap[index] &= ~(byte_t)(1 << bit);
				return true;
			case FlagAction::Set:
				bitmap[index] |= (byte_t)(1 << bit);
				return true;
			case FlagAction::Test:
				return check_flag(bitmap[index], (byte_t)(1 << bit));
			default:
				throw std::runtime_error("Internal error: Invalid switch.");
		}
	}
	template <typename T>
	bool missable_objects_flag_action(FlagAction action, T &bitmap, unsigned bit){
		return flag_action(action, bitmap, bit);
	}
	SpriteStateData1 get_current_sprite1();
	SpriteStateData2 get_current_sprite2();
	void *map_pointer(unsigned pointer);
	void mass_initialization();
	void save_sram(const sram_t &) const;
	void animate_party_mon(bool force_speed_1 = false);
	void handle_down_arrow_blink_timing(const tilemap_it &);
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
	DECLARE_HARDWARE_REGISTER(DIV);

	SystemClock &get_system_clock(){
		return this->clock;
	}
	SoundController &get_sound_controller(){
		return this->sound_controller;
	}
	UserInputController &get_input_controller(){
		return this->input_controller;
	}

	CppRed(HostSystem &host);
	void run();
	void set_bg_scroll(int x = -1, int y = -1);
	void set_window_position(int x = -1, int y = -1);
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
	void clear_both_bg_maps();
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
	void delay3(){
		this->delay_frames(3);
	}
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
	void display_two_option_menu(TwoOptionMenuType type, unsigned x = 0, unsigned y = 0, bool default_to_second_option = false);
	void display_textbox_id(unsigned x = 0, unsigned y = 0);
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
	InputBitmap_struct handle_menu_input();
	InputBitmap_struct handle_menu_input2();
	void joypad();
	void initialize_player_data();
	void initialize_options();
	void call_predef(Predef);
	void add_item_to_inventory(unsigned position, ItemId, unsigned quantity);
	MapId special_warp_in();
	void load_special_warp_data();
	void gb_fadeout_to_white();
	void gb_fadein_from_white();
	void load_front_sprite(SpeciesId, bool flipped, const tilemap_it &destination);
	//Waits until vsync and copies tiles to VRAM.
	//src_offset is the index of the first tile to copy.
	void copy_video_data(const BaseStaticImage &image, unsigned tiles, unsigned src_offset, unsigned destination, bool flipped = false);
	void copy_video_data(const BaseStaticImage &image, unsigned destination, bool flipped = false);
	void copy_video_data(const void *data, size_t size, unsigned destination);
	void reset_player_sprite_data();
	void clear_screen_area(unsigned w, unsigned h, const tilemap_it &location);
	void lcd_stat_irq();
	void vblank_irq();
	const RenderedFrame *get_current_frame();
	void return_used_frame(const RenderedFrame *);
	void toggle_pause(int);
	//Blocks until the next v-blank.
	void delay_frame();
	void wait_for_text_scroll_button_press();
	sram_t load_sram();
	void load_save();
	void place_menu_cursor();
	InputBitmap_struct joypad_low_sensitivity();
	void write_character_at_menu_cursor(byte_t character);
	void erase_menu_cursor();
	void place_unfilled_arrow_menu_cursor();
	void town_map_sprite_blinking_animation();
	void display_picture_centered_or_upper_right(const BaseStaticImage &image, Placing placing);
	std::string display_naming_screen(NamingScreenType);

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
