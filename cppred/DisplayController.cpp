#include "DisplayController.h"
#include "Gameboy.h"
#include "MemoryController.h"
#include "HostSystem.h"
#include "exceptions.h"
#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cassert>

unsigned frames_drawn = 0;

struct SpriteDescription{
	byte_t y, x, tile_no, attributes;
	int get_y() const{
		return (int)this->y - 16U;
	}
	int get_x() const{
		return (int)this->x - 8U;
	}
	int palette_number() const{
		return !!(this->attributes & bit(4));
	}
	bool has_priority() const{
		return !(this->attributes & bit(7));
	}
	bool flipped_x() const{
		return !!(this->attributes & bit(5));
	}
	bool flipped_y() const{
		return !!(this->attributes & bit(6));
	}
};

DisplayController::DisplayController(Gameboy &system):
		system(&system),
		vram(0x2000),
		oam(0xA0),
		display_enabled(false){
	this->set_background_palette(0);
	this->set_obj0_palette(0);
	this->set_obj1_palette(0);
}

RenderedFrame *DisplayController::get_current_frame(){
	return this->publishing_frames.get_public_resource();
}

void DisplayController::return_used_frame(RenderedFrame *frame){
	if (!this->display_enabled)
		this->publishing_frames.return_resource_as_ready(frame);
	else
		this->publishing_frames.return_resource_as_private(frame);
}

int DisplayController::get_row_status(){
	assert(this->display_enabled);
	auto clock = this->get_display_clock();
	auto cycle = (unsigned)(clock % lcd_refresh_period);
	auto row = cycle / 456;
	auto sub_row = cycle % 456;
	row *= 4;
	if (cycle >= lcd_height * 456)
		return row + 3;
	if (sub_row < 80)
		return row + 0;
	if (sub_row < 252)
		return row + 1;
	return row + 2;
}

int DisplayController::get_LY(){
	if (!this->display_enabled)
		return -1;
	auto clock = this->get_display_clock();
	auto cycle = (unsigned)(clock % lcd_refresh_period);
	return cycle / 456;
}

int DisplayController::get_lcd_transfer_status(){
	assert(this->display_enabled);
	auto clock = this->get_display_clock();
	auto cycle = (unsigned)(clock % lcd_refresh_period);
	auto sub_row = (unsigned)(clock % 456);
	if (cycle >= lcd_height * 456)
		return 3;
	if (sub_row < 80)
		return 0;
	if (sub_row < 252)
		return 1;
	return 2;
}

byte_t DisplayController::get_background_palette(){
	return this->bg_palette_value;
}

template <bool BG>
static void set_palette_array(RGB dst[4], byte_t palette){
	for (unsigned i = 4; i--;){
		auto index = (palette >> (i * 2)) & 3;
		byte_t c = ~(byte_t)(index * 0xFF / 3);
		dst[i] = { c, c, c, 0xFF };
	}
	if (!BG)
		dst[0] = { 0, 0, 0, 0 };
}

void DisplayController::set_background_palette(byte_t palette){
	set_palette_array<true>(this->bg_palette, palette);
	this->bg_palette_value = palette;
}

byte_t DisplayController::get_y_coordinate(){
	auto row_status = this->get_LY();
	if (row_status < 0)
		return 0;
	return (byte_t)row_status;
}

byte_t DisplayController::get_status(){
	if (!this->display_enabled)
		return 0;
	auto ret = this->lcd_status & this->stat_writing_filter_mask;
	ret |= (this->get_y_coordinate_compare() == this->get_y_coordinate()) * this->stat_coincidence_flag_mask;
	ret |= ((unsigned)this->get_lcd_transfer_status() + 2) & 3;
	return ret;
}

void DisplayController::set_status(byte_t b){
	b &= this->stat_writing_filter_mask;
	this->lcd_status &= this->stat_comp_writing_filter_mask;
	this->lcd_status |= b;
}

byte_t DisplayController::get_y_coordinate_compare(){
	return this->y_compare;
}

void DisplayController::set_y_coordinate_compare(byte_t b){
	this->y_compare = b;
}

byte_t DisplayController::get_window_x_position(){
	return this->window_x;
}

void DisplayController::set_window_x_position(byte_t b){
	this->window_x = b;
}

byte_t DisplayController::get_window_y_position(){
	return this->window_y;
}

void DisplayController::set_window_y_position(byte_t b){
	this->window_y = b;
}

byte_t DisplayController::get_scroll_x(){
	return (byte_t)this->scroll_x;
}

void DisplayController::set_scroll_x(byte_t b){
	this->scroll_x = b;
}

byte_t DisplayController::get_scroll_y(){
	return (byte_t)this->scroll_y;
}

void DisplayController::set_scroll_y(byte_t b){
	this->scroll_y = b;
}

byte_t DisplayController::get_lcd_control(){
	return this->lcd_control;
}

#define CHECK_FLAG(x) std::cout << (check_flag(this->lcd_control, x) ? " " : "~") << #x "\n"

void DisplayController::set_lcd_control(byte_t b){
	this->lcd_control = b;
	this->toggle_lcd();

	return;
	std::cout << "\nLCDC changed: " << std::hex << (int)this->lcd_control << std::endl;
	CHECK_FLAG(lcdc_display_enable_mask);
	CHECK_FLAG(lcdc_window_map_select_mask);
	CHECK_FLAG(lcdc_window_enable_mask);
	CHECK_FLAG(lcdc_tile_map_select_mask);
	CHECK_FLAG(lcdc_bg_map_select_mask);
	CHECK_FLAG(lcdc_tall_sprite_enable_mask);
	CHECK_FLAG(lcdc_sprite_enable_mask);
	CHECK_FLAG(lcdc_bg_enable_mask);
}

std::uint64_t DisplayController::get_system_clock() const{
	return this->system->get_system_clock().get_clock_value();
}

void DisplayController::toggle_lcd(){
	bool enable = !!(this->lcd_control & lcdc_display_enable_mask);
	if (enable == this->display_enabled)
		return;

	if (enable){
		this->display_clock_start = this->get_system_clock() + 244;
		this->swallow_frames = 1;
	}else{
		this->display_enabled = false;
		this->publishing_frames.clear_public_resource();
		this->last_row_state = -1;
		this->display_clock_start = this->invalid_clock;
		this->enable_memories();
	}
}

byte_t DisplayController::get_obj0_palette(){
	return this->obj0_palette_value;
}

void DisplayController::set_obj0_palette(byte_t palette){
	set_palette_array<false>(this->obj0_palette, palette);
	this->obj0_palette_value = palette;
}

byte_t DisplayController::get_obj1_palette(){
	return this->obj1_palette_value;
}

void DisplayController::set_obj1_palette(byte_t palette){
	set_palette_array<false>(this->obj1_palette, palette);
	this->obj1_palette_value = palette;
}

std::uint64_t DisplayController::get_display_clock() const{
	if (!this->display_enabled)
		return 0;
	return this->get_system_clock() - this->display_clock_start;
}

std::int64_t DisplayController::get_signed_display_clock() const{
	std::int64_t c;
	if (this->display_clock_start == this->invalid_clock)
		c = std::numeric_limits<std::int64_t>::max();
	else
		c = this->display_clock_start;
	return (std::int64_t)this->get_system_clock() - c;
}

bool DisplayController::update(){
	if (!this->display_enabled){
		if (this->display_clock_start > this->get_system_clock())
			return false;
		this->display_enabled = true;
	}
	auto row_status = (unsigned)this->get_row_status();
	auto state = row_status & 3;
	auto row = row_status >> 2;
	if (this->last_row_state == state)
		return false;
	typedef void (DisplayController::*fp_t)(unsigned);
	static const fp_t functions[] = {
		&DisplayController::switch_to_row_state_0,
		&DisplayController::switch_to_row_state_1,
		&DisplayController::switch_to_row_state_2,
		&DisplayController::switch_to_row_state_3,
	};
	(this->*functions[state])(row);
	bool ret = this->last_row_state == 3;
	this->last_row_state = state;
	return ret;
}

void DisplayController::switch_to_row_state_0(unsigned row){
	this->memory_controller->toggle_oam_access(false);
	if (check_flag(this->lcd_status, stat_oam_interrupt_mask))
		this->system->get_cpu().lcd_stat_irq();
}

void DisplayController::switch_to_row_state_1(unsigned row){
	this->memory_controller->toggle_vram_access(false);
	if (this->system->get_mode() == GameboyMode::CGB)
		this->memory_controller->toggle_palette_access(false);
}

void DisplayController::switch_to_row_state_2(unsigned row){
	if (!this->swallow_frames)
		this->render_current_scanline(row);
	this->enable_memories();
	if (check_flag(this->lcd_status, stat_hblank_interrupt_mask))
		this->system->get_cpu().lcd_stat_irq();
}

void DisplayController::enable_memories(){
	this->memory_controller->toggle_oam_access(true);
	this->memory_controller->toggle_vram_access(true);
	if (this->system->get_mode() == GameboyMode::CGB)
		this->memory_controller->toggle_palette_access(true);
}

void DisplayController::switch_to_row_state_3(unsigned row){
	if (!this->swallow_frames){
#ifdef DUMP_FRAMES
		{
			std::stringstream path;
			path << "graphics_output/" << std::setw(5) << std::setfill('0') << frames_drawn << ".bmp";
			this->system->get_host()->write_frame_to_disk(path.str(), *this->frame_being_drawn);
		}
#endif
		this->publishing_frames.publish();
		frames_drawn++;
	}else
		this->swallow_frames--;
	if (check_flag(this->lcd_status, stat_vblank_interrupt_mask))
		this->system->get_cpu().lcd_stat_irq();
	this->system->get_cpu().vblank_irq();
}

struct FullSprite{
	int sprite_number;
	SpriteDescription sprite_description;
	GameboyMode operation_mode;
	//Returns true if *this has less priority than other.
	bool operator<(const FullSprite &other) const{
		if (operation_mode == GameboyMode::CGB)
			return this->sprite_number > other.sprite_number;

		if (this->sprite_description.x > other.sprite_description.x)
			return true;
		if (this->sprite_description.x < other.sprite_description.x)
			return false;
		return this->sprite_number > other.sprite_number;
	}
};

void DisplayController::render_current_scanline(unsigned y){
	auto *pixels = this->publishing_frames.get_private_resource()->pixels;
	const unsigned pitch = lcd_width;

	auto wy = (int)this->window_y;
	auto bg_tile_vram = this->get_bg_tile_vram();
	auto tile_no_offset = this->get_tile_no_offset();
	auto bg_vram = this->get_bg_vram();
	auto window_vram = this->get_window_vram();
	auto oam = this->get_oam();
	auto sprite_tile_vram = this->get_sprite_tile_vram();
	bool bg_enabled = check_flag(this->lcd_control, lcdc_bg_enable_mask);
	bool sprites_enabled = check_flag(this->lcd_control, lcdc_sprite_enable_mask);
	const int sprite_width = 8;
	bool tall_sprites = check_flag(this->lcd_control, lcdc_tall_sprite_enable_mask);
	int sprite_height = tall_sprites ? 16 : 8;

	auto row = pixels + pitch * y;
	auto src_y = (y + this->scroll_y) & 0xFF;
	auto src_y_prime = src_y / 8 * 32;

	auto wy_prime = (int)y - wy;
	bool window_enabled = check_flag(this->lcd_control, lcdc_window_enable_mask) && wy_prime >= 0 && wy_prime < lcd_height;
	auto y_prime = wy_prime / 8 * 32;

	RGB *obj_palettes[] = {
		this->obj0_palette,
		this->obj1_palette,
	};

	FullSprite sprites_for_scanline[40];
	unsigned sprites_for_scanline_size = 0;
	auto operation_mode = this->system->get_mode();
	if (sprites_enabled){
		for (unsigned i = 40; i--;){
			auto &sprite = *(SpriteDescription *)(oam + i * 4);
			auto spry = sprite.get_y();
			if ((int)y >= spry && (int)y < spry + sprite_height){
				sprites_for_scanline[sprites_for_scanline_size].sprite_number = i;
				sprites_for_scanline[sprites_for_scanline_size].sprite_description = sprite;
				sprites_for_scanline[sprites_for_scanline_size].operation_mode = operation_mode;
				sprites_for_scanline_size++;
			}
		}
	}

	FullSprite *sprites = sprites_for_scanline;

	if (sprites_for_scanline_size){
		std::sort(sprites_for_scanline, sprites_for_scanline + sprites_for_scanline_size);
		if (sprites_for_scanline_size > 10){
			sprites += sprites_for_scanline_size - 10;
			sprites_for_scanline_size = 10;
		}
	}

	for (unsigned x = 0; x != lcd_width; x++){
		unsigned color_index = 0;
		RGB *palette = nullptr;
		int source = -1;

		if (window_enabled){
			auto wx = (int)this->window_x - 7;
			auto src_x = (int)x - wx;
			if ((src_x >= 0) & (src_x < (int)lcd_width)){
				auto src_window_tile = src_x / 8 + y_prime;
				byte_t tile_no = window_vram[src_window_tile] + tile_no_offset;
				auto tile = bg_tile_vram + tile_no * 16;
				auto tile_offset_x = src_x & 7;
				auto tile_offset_y = wy_prime & 7;
				auto src_pixelA = tile[tile_offset_y * 2 + 0];
				auto src_pixelB = tile[tile_offset_y * 2 + 1];
				color_index = (src_pixelA >> (7 - tile_offset_x) & 1) | (((src_pixelB >> (7 - tile_offset_x)) & 1) << 1);
				palette = this->bg_palette;
				source = 0;
			}
		}

		if (bg_enabled & !palette){
			auto src_x = (x + this->scroll_x) & 0xFF;
			auto src_bg_tile = src_x / 8 + src_y_prime;
			byte_t tile_no = bg_vram[src_bg_tile] + tile_no_offset;
			auto tile = bg_tile_vram + tile_no * 16;
			auto tile_offset_x = ~src_x & 7;
			auto tile_offset_y = (src_y & 7) << 1;
			auto src_pixelA = tile[tile_offset_y | 0];
			auto src_pixelB = tile[tile_offset_y | 1];
			auto first_part = (src_pixelA >> tile_offset_x) & 1;
			auto second_part = ((src_pixelB >> tile_offset_x) & 1) << 1;
			color_index = first_part | second_part;
			palette = this->bg_palette;
			source = 1;
		}

		for (unsigned i = 0; i != sprites_for_scanline_size; i++){
			auto sprite = sprites[i].sprite_description;
			auto sprx = sprite.get_x();
			auto sprite_is_here = ((int)x >= sprx) & ((int)x < sprx + sprite_width);
			auto sprite_is_not_covered_here = sprite.has_priority() | !color_index;
			if (!(sprite_is_here & sprite_is_not_covered_here))
				continue;

			byte_t tile_no = sprite.tile_no;
			auto spry = sprite.get_y();
			auto tile_offset_x = (int)x - sprx;
			auto tile_offset_y = (int)y - spry;
			tile_offset_x ^= 7 * !sprite.flipped_x();
			tile_offset_y ^= 7 * sprite.flipped_y();

			if (tall_sprites)
				tile_no &= 0xFE;
			tile_offset_y <<= 1;
			auto tile = sprite_tile_vram + tile_no * 16;
			auto src_pixelA = tile[tile_offset_y | 0];
			auto src_pixelB = tile[tile_offset_y | 1];
			auto first_part = (src_pixelA >> tile_offset_x) & 1;
			auto second_part = ((src_pixelB >> tile_offset_x) & 1) << 1;
			auto index = first_part | second_part;
			if (!index)
				continue;

			color_index = index;
			palette = obj_palettes[sprite.palette_number()];
			source = 2;

			break;
		}
		if (palette){
			auto &pixel = row[x];
			pixel = palette[color_index];
#ifdef DEBUG_FRAMES
			if (source != 0)
				pixel.r = 0;
			if (source != 1)
				pixel.g = 0;
			if (source != 2)
				pixel.b = 0;
#endif
		}else
			row[x] = { 0xFF, 0xFF, 0xFF, 0xFF };
	}
}

std::ostream &operator<<(std::ostream &stream, const RGB &rgb){
	stream << std::hex;
	stream << std::setw(2) << std::setfill('0') << (int)rgb.r;
	stream << std::setw(2) << std::setfill('0') << (int)rgb.g;
	stream << std::setw(2) << std::setfill('0') << (int)rgb.b;
	stream << std::setw(2) << std::setfill('0') << (int)rgb.a;
	stream << std::dec;
	return stream;
}
