#include "CppRed.h"
#include "CppRedTitleScreen.h"
#include "HostSystem.h"
#include "timer.h"
#include "../CodeGeneration/output/gfx.h"
#include "CppRedData.h"
#include "CppRedClearSave.h"
#include "CppRedMainMenu.h"
#include <cassert>
#include <stdexcept>

#define INITIALIZE_EMPTY_HARDWARE_REGISTER(name) \
	,name(nullptr, {[this](byte_t &dst, const void *){}, [this](const byte_t &src, void *){}})
#define INITIALIZE_HARDWARE_REGISTER(name, controller, function) \
	,name(nullptr, {[this](byte_t &dst, const void *){ dst = this->controller.get_##function(); }, [this](const byte_t &src, void *){ this->controller.set_##function(src); }})
#define INITIALIZE_HARDWARE_REGISTER_RO(name, controller, function) \
	,name(nullptr, {[this](byte_t &dst, const void *){ dst = this->controller.get_##function(); }, [this](const byte_t &src, void *){}})

CppRed::CppRed(HostSystem &host):
		host(&host),
		display_controller(*this),
		sound_controller(*this),
		input_controller(*this),
		clock(*this),
		audio(*this),
		continue_running(false)
		INITIALIZE_EMPTY_HARDWARE_REGISTER(SB)
		INITIALIZE_EMPTY_HARDWARE_REGISTER(SC)
		INITIALIZE_HARDWARE_REGISTER(TMA, clock, TMA_register)
		INITIALIZE_HARDWARE_REGISTER(TAC, clock, TAC_register)
		INITIALIZE_EMPTY_HARDWARE_REGISTER(BGP)
		INITIALIZE_EMPTY_HARDWARE_REGISTER(OBP0)
		INITIALIZE_EMPTY_HARDWARE_REGISTER(OBP1)
		INITIALIZE_HARDWARE_REGISTER(LCDC, display_controller, lcd_control)
		INITIALIZE_HARDWARE_REGISTER(STAT, display_controller, status)
		INITIALIZE_EMPTY_HARDWARE_REGISTER(IF)
		INITIALIZE_EMPTY_HARDWARE_REGISTER(IE)
		INITIALIZE_EMPTY_HARDWARE_REGISTER(InterruptMasterFlag)
		INITIALIZE_HARDWARE_REGISTER(SCX, display_controller, scroll_x)
		INITIALIZE_HARDWARE_REGISTER(SCY, display_controller, scroll_y)
		INITIALIZE_HARDWARE_REGISTER(WX, display_controller, window_x_position)
		INITIALIZE_HARDWARE_REGISTER(WY, display_controller, window_y_position)
		INITIALIZE_HARDWARE_REGISTER_RO(LY, display_controller, y_coordinate)
		INITIALIZE_HARDWARE_REGISTER(LYC, display_controller, y_coordinate_compare)
		{

	this->nonemulation_init();
}

void CppRed::nonemulation_init(){
	this->realtime_counter_frequency = get_timer_resolution();
	//this->emulated_memory.reset(new byte_t[0x10000]);
}

void CppRed::init(){
	this->IF = 0;
	this->IE = 0;
	this->set_bg_scroll(0, 0);
	this->set_window_position(0, 0);
	this->SB = 0;
	this->SC = 0;
	this->TMA = 0;
	this->TAC = 0;
	this->BGP = 0;
	this->OBP0 = 0;
	this->OBP1 = 0;
	//Enable display:
	this->LCDC = DisplayController::lcdc_display_enable_mask;

	this->disable_lcd();

	this->clear_wram();
	this->clear_vram();
	this->clear_hram();
	this->clear_sprites();

	//At this point, a DMA transfer routine is copied to HRAM. Since we can
	//execute stuff whenever we want, we don't need to do this.

	this->hram.hTilesetType = 0;
	this->STAT = 0;
	this->hram.hSCX = 0;
	this->hram.hSCY = 0;

	this->IF = 0;
	this->IE = vblank_mask | timer_mask | serial_mask;
	this->hram.hWY = lcd_height;
	this->set_window_position(7, lcd_height);

	this->hram.hSerialConnectionStatus = SerialConnectionStatus::ConnectionNotEstablished;

	this->clear_bg_map(bg_map0 >> 8);
	this->clear_bg_map(bg_map1 >> 8);

	this->LCDC = lcdc_default;

	this->hram.hSoftReset = 16;
	this->stop_all_sounds();

	this->InterruptMasterFlag = 1;

	this->execute_predef(Predef::LoadSGB);

	this->hram.H_AUTOBGTRANSFERDEST = 0x9C00;
	this->wram.wUpdateSpritesEnabled = 0xFF;

	this->execute_predef(Predef::PlayIntro);

	this->disable_lcd();
	this->clear_vram();
	this->gb_pal_normal();
	this->clear_sprites();
	
	this->LCDC = lcdc_default;

	this->set_default_names_before_titlescreen();
}

void CppRed::clear_wram(){
	this->wram.clear();
}

void CppRed::clear_hram(){
	this->hram.clear();
}

void CppRed::clear_sprites(){
	this->wram.wOAMBuffer.fill(0);
}

void CppRed::execute_predef(Predef predef){
	this->predef_functions[(int)predef]();
}

const std::string ninten_text = "NINTEN";
const std::string sony_text = "SONY";

void CppRed::set_default_names_before_titlescreen(){
	this->wram.wPlayerName = ninten_text;
	this->wram.wRivalName = sony_text;
	this->set_window_position(-1, 0);
	this->wram.wLetterPrintingDelayFlags = 0;
	this->wram.wd732.clear();
	this->wram.wFlags_D733.clear();
	this->wram.wBeatLorelei.clear();
	this->wram.wAudioROMBank.value = 0;
	this->wram.wAudioSavedROMBank.value = 0;
}

TitleScreenResult CppRed::display_titlescreen(){
	CppRedTitleScreen title_screen(*this);
	return title_screen.display();
}

void CppRed::load_copyright_graphics(){
	//TODO:
	/*
	 * ld hl, NintendoCopyrightLogoGraphics
	 * ld de, vTitleLogo2 + $100
	 * ld bc, $50
	 * ld a, BANK(NintendoCopyrightLogoGraphics)
	 * call FarCopyData2
	 */
}

void CppRed::load_gamefreak_logo(){
	//TODO:
	/*
	 * ld hl, GamefreakLogoGraphics
	 * ld de, vTitleLogo2 + $100 + $50
	 * ld bc, $90
	 * ld a, BANK(GamefreakLogoGraphics)
	 * call FarCopyData2
	 */
}

void CppRed::load_pokemon_logo(){
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

void CppRed::load_version_graphics(){
	//TODO:
	/*
	 * ld hl, Version_GFX
	 * ld de,vChars2 + $600 - (Version_GFXEnd - Version_GFX - $50)
	 * ld bc, Version_GFXEnd - Version_GFX
	 * ld a, BANK(Version_GFX)
	 * call FarCopyDataDouble
	 */
}

void CppRed::run(){
	if (this->continue_running)
		return;
	if (!this->registered){
		this->registered = true;
		this->host->get_timing_provider()->register_periodic_notification(this->periodic_notification);
	}
	this->continue_running = true;
	auto This = this;
	this->interpreter_thread.reset(new std::thread([This](){ This->interpreter_thread_function(); }));
}

void CppRed::interpreter_thread_function(){
	if (this->accumulated_time < 0)
		this->accumulated_time = 0;
	if (!this->start_time.is_initialized())
		this->start_time = this->host->get_datetime_provider()->local_now().to_posix();

	std::shared_ptr<std::exception> thrown;

	try{
#if 1
		this->main();
#else
		while (true){
			bool continue_running;
			bool paused;
			this->real_time_multiplier = this->speed_multiplier / (double)this->realtime_counter_frequency;
			this->current_timer_start = get_timer_count();
			while (true){
				continue_running = this->continue_running;
				paused = this->paused;
				if (!continue_running || paused)
					break;

				auto t0 = get_timer_count();
				this->run_until_next_frame();
				//this->ram_to_save.try_save(*this->host);
				auto t1 = get_timer_count();
#ifndef BENCHMARKING
				this->sync_with_real_time();
#endif
				auto t2 = get_timer_count();

				this->time_running += t1 - t0;
				this->time_waiting += t2 - t1;
			}
			if (!continue_running)
				break;
			this->accumulated_time = this->get_real_time();
			if (paused)
				this->execute_pause();
		}
#endif
	}catch (GameBoyException &ex){
		thrown.reset(ex.clone());
	}catch (...){
		thrown.reset(new GenericException("Unknown exception."));
	}

	this->host->throw_exception(thrown);
}

#if 0
void CppRed::run_until_next_frame(){
	do{
		this->cpu.run_one_instruction();
		if (this->input_controller.get_button_down())
			this->cpu.joystick_irq();
		this->sound_controller.update(this->speed_multiplier, this->speed_changed);
		this->speed_changed = false;
	}while (!this->display_controller.update() && (this->continue_running || force));
}
#endif

void CppRed::main(){
	MainMenuResult main_menu_result;
	while (true){
		this->init();
		bool done = false;
		while (!done){
			switch (this->display_titlescreen()){
				case TitleScreenResult::GoToMainMenu:
					main_menu_result = this->display_main_menu();
					done = main_menu_result != MainMenuResult::Cancelled;
					break;
				case TitleScreenResult::GoToClearSaveDialog:
					this->display_clear_save_dialog();
					done = true;
					break;
			}
		}
		if (main_menu_result != MainMenuResult::Cancelled)
			break;
	}
	assert(main_menu_result != MainMenuResult::Cancelled);
	switch (main_menu_result){
		case MainMenuResult::NewGame:
			this->start_new_game();
			break;
		case MainMenuResult::ContinueGame:
			this->start_loaded_game();
			break;
		default:
			throw std::runtime_error("Invalid switch value.");
	}
}

void CppRed::set_bg_scroll(int x, int y){
	if (x >= 0)
		this->SCX = (byte_t)x;
	if (y >= 0)
		this->SCY = (byte_t)y;
}

void CppRed::set_window_position(int x, int y){
	if (x >= 0)
		this->WX = (byte_t)x;
	if (y >= 0)
		this->WY = (byte_t)y;
}

void CppRed::enable_lcd(){
	this->LCDC |= DisplayController::lcdc_display_enable_mask;
}

void CppRed::disable_lcd(){
	this->IF = 0;
	auto ie = this->IE;
	this->IE = ie & ~vblank_mask;

	//TODO: Replace spinlock with a wait or something:
	while (this->LY != lcd_blank);
	
	this->LCDC &= ~DisplayController::lcdc_display_enable_mask;

	this->IE = ie;
}

void CppRed::clear_vram(){
	this->display_controller.clear_vram();
}

void CppRed::clear_bg_map(unsigned page){
	page <<= 8;
	//TODO: This can be optimized.
	auto vmem = &this->display_controller.access_vram(page);
	memset(vmem, 0, 0x400);
}

void CppRed::gb_pal_normal(){
	this->BGP = 0xE4;
	this->OBP0 = 0xD0;
}

void CppRed::gb_pal_whiteout(){
	this->BGP = 0;
	this->OBP0 = 0;
	this->OBP1 = 0;
}

void CppRed::clear_screen(){
	auto begin = this->wram.wTileMap.begin();
	std::fill(begin, begin + this->wram.wTileMap.size, 0x7F);
}

void CppRed::load_font_tile_patterns(){
	auto font_graphics = decode_image_data(FontGraphics);
	auto dst = &this->display_controller.access_vram(vFont);
	memcpy(dst, &font_graphics[0], font_graphics.size());
}

void CppRed::clear_both_bg_maps(){
	auto bg = &this->display_controller.access_vram(bg_map0);
		memset(bg, 0x7F, 0x800);
}

void CppRed::save_screen_tiles_to_buffer2(){
	auto src = this->get_tilemap_location(0, 0);
	const auto size = this->wram.wTileMapBackup2.size;
	auto dst = this->wram.wTileMapBackup2.begin();
	std::copy(src, src + size, dst);
}

void CppRed::load_screen_tiles_from_buffer1(){
	this->hram.H_AUTOBGTRANSFERENABLED = 0;
	auto src = this->wram.wTileMapBackup.begin();
	const auto size = this->wram.wTileMapBackup.size;
	auto dst = this->get_tilemap_location(0, 0);
	std::copy(src, src + size, dst);
	this->hram.H_AUTOBGTRANSFERENABLED = 1;
}

void CppRed::load_screen_tiles_from_buffer2(){
	this->load_screen_tiles_from_buffer2_disable_bg_transfer();
	this->hram.H_AUTOBGTRANSFERENABLED = 1;
}

void CppRed::load_screen_tiles_from_buffer2_disable_bg_transfer(){
	this->hram.H_AUTOBGTRANSFERENABLED = 0;
	auto src = this->wram.wTileMapBackup2.begin();
	const auto size = this->wram.wTileMapBackup2.size;
	auto dst = this->get_tilemap_location(0, 0);
	std::copy(src, src + size, dst);
}

void CppRed::save_screen_tiles_to_buffer1(){
	auto src = this->get_tilemap_location(0, 0);
	const auto size = this->wram.wTileMapBackup2.size;
	auto dst = this->wram.wTileMapBackup2.begin();
	std::copy(src, src + size, dst);
}

void CppRed::run_palette_command(PaletteCommand cmd){
	if (!this->wram.wOnSGB)
		return;
	this->execute_predef(Predef::_RunPaletteCommand);
}

void CppRed::delay_frames(unsigned count){
	while (count--)
		this->display_controller.wait_for_vsync();
}

void CppRed::stop_all_sounds(){
	this->wram.wAudioROMBank = AudioBank::Bank1;
	this->wram.wAudioSavedROMBank = AudioBank::Bank1;
	this->wram.wAudioFadeOutControl = 0;
	this->wram.wNewSoundID = Sound::None;
	this->wram.wLastMusicSoundID = Sound::None;
	this->play_sound(Sound::Stop);
}

void CppRed::play_sound(Sound sound){
	if (this->wram.wNewSoundID.value){
		auto i = this->wram.wChannelSoundIDs.begin() + 4;
		std::fill(i, i + 4, 0);
	}
	if (this->wram.wAudioFadeOutControl){
		if (!this->wram.wNewSoundID.value)
			return;
		this->wram.wNewSoundID.value = 0;
		if (this->wram.wLastMusicSoundID.value != 0xFF){
			this->wram.wLastMusicSoundID = sound;

			this->wram.wAudioFadeOutCounter =
			this->wram.wAudioFadeOutCounterReloadValue = this->wram.wAudioFadeOutControl;

			//TODO: What does this mean?
			this->wram.wAudioFadeOutControl = (unsigned)sound;
			return;
		}
		this->wram.wAudioFadeOutControl = 0;
	}
	this->wram.wNewSoundID.value = 0;
	switch (this->wram.wAudioROMBank){
		case AudioBank::Bank1:
			this->audio.audio1_play_sound(sound);
			break;
		case AudioBank::Bank2:
			this->audio.audio2_play_sound(sound);
			break;
		case AudioBank::Bank3:
			this->audio.audio3_play_sound(sound);
			break;
		default:
			throw std::runtime_error("Invalid audio bank value.");
	}
}

void CppRed::delay3(){
	this->delay_frames(3);
}

void CppRed::wait_for_sound_to_finish(){
	if (check_flag<byte_t>(this->wram.wLowHealthAlarm, 0x80))
		return;
	byte_t a;
	//TODO: Replace spinlock with a wait or something:
	//TODO: VERY IMPORTANT: This spinlock can never break!
	do{
		a = this->wram.wChannelSoundIDs[4];
		a |= this->wram.wChannelSoundIDs[5];
		a |= this->wram.wChannelSoundIDs[7];
	}while (a);
}

void CppRed::play_cry(SpeciesId species){
	this->play_sound(this->get_cry_data(species));
	this->wait_for_sound_to_finish();
}

Sound CppRed::get_cry_data(SpeciesId species){
	auto index = (unsigned)species - 1;
	assert(index < pokemon_cry_data_size);
	auto &data = pokemon_cry_data[index];
	this->wram.wFrequencyModifier = data.pitch;
	this->wram.wTempoModifier = data.length;
	return (Sound)(data.base_cry + (unsigned)Sound::SFX_Cry00_1);
}

void CppRed::load_gb_pal(){
	int offset = this->wram.wMapPalOffset;
	const byte_t *data = (const byte_t *)fade_palettes;
	int index = 3 * (int)sizeof(FadePaletteData) - offset;
	assert(index >= 0 && index < sizeof(fade_palettes) - 3);
	auto palette = *(const FadePaletteData *)(data + index);
	this->BGP = palette.background_palette;
	this->OBP0 = palette.obp0_palette;
	this->OBP1 = palette.obp1_palette;
}

void CppRed::display_clear_save_dialog(){
	CppRedClearSaveDialog dialog(*this);
	if (dialog.display())
		this->clear_save();
}

MainMenuResult CppRed::display_main_menu(){
	CppRedMainMenu dialog(*this);
	return dialog.display();
}

void CppRed::start_loaded_game(){
	this->gb_pal_white_out_with_delay3();
	this->clear_screen();
	this->wram.wPlayerDirection = PlayerDirectionBitmap::Right;
	this->delay_frames(10);
	if (!this->wram.wNumHoFTeams){
		this->special_enter_map(MapId::PalletTown);
		return;
	}
	auto current_map = this->wram.wCurMap.enum_value();
	if (current_map != MapId::HallOfFame){
		this->special_enter_map(current_map);
		return;
	}
	this->wram.wDestinationMap = MapId::PalletTown;
	this->wram.wd732.set_fly_warp(true);
	this->special_enter_map(this->special_warp_in());
}

void CppRed::print_text(const CppRedText::Region &region){
	this->text.print_text(region);
}

void CppRed::update_sprites(){
	if (this->wram.wUpdateSpritesEnabled != 1)
		return;

	unsigned i = 0;
	for (const auto &spr : this->wram.wSpriteStateData2){
		this->hram.H_CURRENTSPRITEOFFSET = i++ * SpriteStateData1::size;
		if (!spr.sprite_image_base_offset)
			continue;
		if (spr.sprite_image_base_offset == 1)
			this->update_player_sprite();
		else
			this->update_non_player_sprite(spr);
	}
}

CppRed::tilemap_it CppRed::get_tilemap_location(unsigned x, unsigned y){
	if (x > tilemap_width || y > tilemap_height){
		std::stringstream stream;
		stream << "Invalid coordinates: " << x << ", " << y;
		throw std::runtime_error(stream.str());
	}
	return this->wram.wTileMap.begin() + y * tilemap_width + x;
}

void CppRed::update_player_sprite(){
	//wSpriteStateData2[0] is always the PC sprite.
	auto sprite1 = this->wram.wSpriteStateData1[0];
	auto sprite2 = this->wram.wSpriteStateData2[0];
	byte_t counter = sprite2.walk_animation_counter;
	bool disable_sprite = true;
	if (!counter){
		auto p = this->get_tilemap_location(8, 9);
		byte_t tile = *p;
		this->hram.hTilePlayerStandingOn = tile;
		disable_sprite = tile >= 0x60;
	}else{
		if (counter != 0xFF){
			counter--;
			sprite2.walk_animation_counter = counter;
		}
	}
	if (disable_sprite){
		sprite1.sprite_image_idx = 0xFF;
		return;
	}

	this->detect_sprite_collision();

	bool not_moving = false;
	if (!this->wram.wWalkCounter){
		PlayerDirectionBitmap pc_direction = this->wram.wPlayerMovingDirection;
		SpriteFacingDirection sprite_direction = SpriteFacingDirection::Down;
		if (check_flag((unsigned)pc_direction, (unsigned)PlayerDirectionBitmap::Down)){
			sprite_direction = SpriteFacingDirection::Down;
		}else if (check_flag((unsigned)pc_direction, (unsigned)PlayerDirectionBitmap::Up)){
			sprite_direction = SpriteFacingDirection::Up;
		}else if (check_flag((unsigned)pc_direction, (unsigned)PlayerDirectionBitmap::Left)){
			sprite_direction = SpriteFacingDirection::Left;
		}else if (check_flag((unsigned)pc_direction, (unsigned)PlayerDirectionBitmap::Right)){
			sprite_direction = SpriteFacingDirection::Right;
		}else
			not_moving = true;

		if (!not_moving){
			sprite1.facing_direction = sprite_direction;
			not_moving = check_flag((byte_t)this->wram.wFontLoaded, 1);
		}
	}

	bool skip_sprite_animation = false;
	if (not_moving){
		sprite1.intra_anim_frame_counter = 0;
		sprite1.anim_frame_counter = 0;
	}else{
		skip_sprite_animation = this->wram.wd736.get_pc_spinning();
		if (!skip_sprite_animation){
			auto sprite = this->wram.wSpriteStateData1[(this->hram.H_CURRENTSPRITEOFFSET / SpriteStateData1::size) + 1];
			if (++sprite.intra_anim_frame_counter != 4)
				skip_sprite_animation = true;
			else{
				sprite.intra_anim_frame_counter = 0;
				sprite.anim_frame_counter = (sprite.anim_frame_counter + 1) % 4;
			}
		}
	}

	if (!skip_sprite_animation)
		sprite1.sprite_image_idx = sprite1.anim_frame_counter + (unsigned)(SpriteFacingDirection)sprite1.facing_direction;

	//If the player is standing on a grass tile, make the player's sprite have
	//lower priority than the background so that it's partially obscured by the
	//grass.Only the lower half of the sprite is permitted to have the priority
	//bit set by later logic.
	sprite2.grass_priority = 0x80 * (this->hram.hTilePlayerStandingOn == this->wram.wGrassTile);
}

void CppRed::update_non_player_sprite(const SpriteStateData2 &sprite){
	unsigned weird = sprite.sprite_image_base_offset + 1;

	//Swap nibbles of weird:
	weird &= 0xFF;
	weird = (weird << 4) | (weird >> 4);
	weird &= 0xFF;
	
	this->hram.hTilePlayerStandingOn = weird;

	if (this->wram.wNPCMovementScriptSpriteOffset == this->hram.H_CURRENTSPRITEOFFSET)
		this->do_scripted_npc_movement();
	else
		this->update_npc_sprite(sprite);
}

void CppRed::do_scripted_npc_movement(){
	//This is an alternative method of scripting an NPC's movement and is only used
	//a few times in the game. It is used when the NPC and player must walk together
	//in sync, such as when the player is following the NPC somewhere. An NPC can't
	//be moved in sync with the player using the other method.

	if (!this->wram.wd730.get_input_simulated())
		return;

	bool initialized = this->wram.wd72e.get_npc_movement_initialized();
	if (!initialized){
		this->wram.wd72e.set_npc_movement_initialized(true);
		this->initialize_scripted_npc_movement();
		return;
	}

	auto direction = (NpcMovementDirection)this->wram.wNPCMovementDirections2[this->wram.wNPCMovementDirections2Index];
	std::unique_ptr<SpriteStateData1::member_type> pointer;
	auto sprite = this->wram.wSpriteStateData1[this->hram.H_CURRENTSPRITEOFFSET / SpriteStateData1::size];
	SpriteFacingDirection sprite_direction = SpriteFacingDirection::Down;
	int displacement = 0;
	switch (direction){
		case NpcMovementDirection::Down:
			pointer = copy_to_unique(sprite.y_pixels);
			sprite_direction = SpriteFacingDirection::Down;
			displacement = 2;
			break;
		case NpcMovementDirection::Up:
			pointer = copy_to_unique(sprite.y_pixels);
			sprite_direction = SpriteFacingDirection::Up;
			displacement = -2;
			break;
		case NpcMovementDirection::Left:
			pointer = copy_to_unique(sprite.x_pixels);
			sprite_direction = SpriteFacingDirection::Left;
			displacement = -2;
			break;
		case NpcMovementDirection::Right:
			pointer = copy_to_unique(sprite.x_pixels);
			sprite_direction = SpriteFacingDirection::Right;
			displacement = 2;
			break;
		default:
			//Is this an error condition?
			return;
	}

	*pointer += displacement;
	sprite.facing_direction = sprite_direction;
	this->anim_scripted_npc_movement();
	if (--this->wram.wScriptedNPCWalkCounter)
		return;
	this->wram.wScriptedNPCWalkCounter = 8;
	this->wram.wNPCMovementDirections2Index++;
}
