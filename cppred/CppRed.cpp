#include "CppRed.h"
#include "CppRedTitleScreen.h"
#include "HostSystem.h"
#include "timer.h"
#include "../CodeGeneration/output/gfx.h"
#include "CppRedData.h"
#include "CppRedClearSave.h"
#include "CppRedMainMenu.h"
#include "CppRedScripts.h"
#include <cassert>
#include <stdexcept>
#include <random>

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
		INITIALIZE_HARDWARE_REGISTER_RO(DIV, clock, DIV_register)
		{

	this->nonemulation_init();
}

void CppRed::nonemulation_init(){
	this->realtime_counter_frequency = get_timer_resolution();
	//this->emulated_memory.reset(new byte_t[0x10000]);

	std::random_device rnd;
	for (auto &i : this->xorshift_state)
		i = rnd();

	this->predefs.resize((int)Predef::COUNT);
	this->predefs[(int)Predef::IsObjectHidden] = [this](){ this->is_object_hidden(); };
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
	MainMenuResult main_menu_result = MainMenuResult::Cancelled;
	while (main_menu_result == MainMenuResult::Cancelled){
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
	}
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
	auto index = (unsigned)species;
	assert(index < array_length(pokemon_by_species_id));
	auto &data = pokemon_by_species_id[index]->cry_data;
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

CppRed::tilemap_it CppRed::get_tilemap_location(unsigned x, unsigned y){
	if (x > tilemap_width || y > tilemap_height){
		std::stringstream stream;
		stream << "Invalid coordinates: " << x << ", " << y;
		throw std::runtime_error(stream.str());
	}
	return this->wram.wTileMap.begin() + y * tilemap_width + x;
}

std::uint32_t xorshift128(std::uint32_t state[4]){
	auto x = state[3];
	x ^= x << 11;
	x ^= x >> 8;
	state[3] = state[2];
	state[2] = state[1];
	state[1] = state[0];
	x ^= state[0];
	x ^= state[0] >> 19;
	state[0] = x;
	return x;
}

unsigned CppRed::random(){
	return xorshift128(this->xorshift_state);
}

void CppRed::call_predef(Predef f){
	this->predefs[(int)f]();
}

void CppRed::start_new_game(){
	//Do not skip name selection.
	this->wram.wd732.set_unknown(false);
	CppRedScripts::oak_speech(*this);
	this->delay_frames(20);
	//TODO: Confirm this. StartNewGame falls through to SpecialEnterMap, but I
	//don't know the state of the registers. I'm assuming this is what happens.
	this->special_enter_map(MapId::RedsHouse2f);
}

void CppRed::mass_initialization(){
	this->wram.wPlayerName.fill_bytes(0);
	this->wram.wPartyCount = 0;
	this->wram.wPartySpecies.fill_bytes(0);
	this->wram.wPartyMons.fill_bytes(0);
	this->wram.wPartyMonOT.fill_bytes(0);
	this->wram.wPartyMonNicks.fill_bytes(0);
	this->wram.wPokedexOwned.fill_bytes(0);
	this->wram.wPokedexSeen.fill_bytes(0);
	this->wram.wNumBagItems = 0;
	this->wram.wBagItems.fill_bytes(0);
	this->wram.wBagItemsTerminator = 0;
	this->wram.wPlayerMoney = 0;
	this->wram.wRivalName.fill_bytes(0);
	this->wram.wOptions.clear();
	this->wram.wObtainedBadges.fill_bytes(0);
	this->wram.wLetterPrintingDelayFlags = 0;
	this->wram.wPlayerID = 0;
	this->wram.wMapMusicSoundID = 0;
	this->wram.wMapMusicROMBank = 0;
	this->wram.wMapPalOffset = 0;
	this->wram.wCurMap = MapId::PalletTown;
	this->wram.wCurrentTileBlockMapViewPointer = 0;
	this->wram.wYCoord = 0;
	this->wram.wXCoord = 0;
	this->wram.wYBlockCoord = 0;
	this->wram.wXBlockCoord = 0;
	this->wram.wLastMap = 0;
	this->wram.wUnusedD366 = 0;
	this->wram.wCurMapTileset = 0;
	this->wram.wCurMapHeight = 0;
	this->wram.wCurMapWidth = 0;
	this->wram.wMapDataPtr = 0;
	this->wram.wMapTextPtr = 0;
	this->wram.wMapScriptPtr = 0;
	this->wram.wMapConnections = 0;
	this->wram.wMapConn1Ptr = 0;
	this->wram.wNorthConnectionStripSrc = 0;
	this->wram.wNorthConnectionStripDest = 0;
	this->wram.wNorthConnectionStripWidth = 0;
	this->wram.wNorthConnectedMapWidth = 0;
	this->wram.wNorthConnectedMapYAlignment = 0;
	this->wram.wNorthConnectedMapXAlignment = 0;
	this->wram.wNorthConnectedMapViewPointer = 0;
	this->wram.wMapConn2Ptr = 0;
	this->wram.wSouthConnectionStripSrc = 0;
	this->wram.wSouthConnectionStripDest = 0;
	this->wram.wSouthConnectionStripWidth = 0;
	this->wram.wSouthConnectedMapWidth = 0;
	this->wram.wSouthConnectedMapYAlignment = 0;
	this->wram.wSouthConnectedMapXAlignment = 0;
	this->wram.wSouthConnectedMapViewPointer = 0;
	this->wram.wMapConn3Ptr = 0;
	this->wram.wWestConnectionStripSrc = 0;
	this->wram.wWestConnectionStripDest = 0;
	this->wram.wWestConnectionStripHeight = 0;
	this->wram.wWestConnectedMapWidth = 0;
	this->wram.wWestConnectedMapYAlignment = 0;
	this->wram.wWestConnectedMapXAlignment = 0;
	this->wram.wWestConnectedMapViewPointer = 0;
	this->wram.wMapConn4Ptr = 0;
	this->wram.wEastConnectionStripSrc = 0;
	this->wram.wEastConnectionStripDest = 0;
	this->wram.wEastConnectionStripHeight = 0;
	this->wram.wEastConnectedMapWidth = 0;
	this->wram.wEastConnectedMapYAlignment = 0;
	this->wram.wEastConnectedMapXAlignment = 0;
	this->wram.wEastConnectedMapViewPointer = 0;
	this->wram.wSpriteSet.fill_bytes(0);
	this->wram.wSpriteSetID = 0;
	this->wram.wObjectDataPointerTemp.fill_bytes(0);
	this->wram.wMapBackgroundTile = 0;
	this->wram.wNumberOfWarps = 0;
	this->wram.wWarpEntries.fill_bytes(0);
	this->wram.wDestinationWarpID = 0;
	this->wram.wNumSigns = 0;
	this->wram.wSignCoords.fill_bytes(0);
	this->wram.wSignTextIDs.fill_bytes(0);
	this->wram.wNumSprites = 0;
	this->wram.wYOffsetSinceLastSpecialWarp = 0;
	this->wram.wXOffsetSinceLastSpecialWarp = 0;
	this->wram.wMapSpriteData.fill_bytes(0);
	this->wram.wMapSpriteExtraData.fill_bytes(0);
	this->wram.wCurrentMapHeight2 = 0;
	this->wram.wCurrentMapWidth2 = 0;
	this->wram.wMapViewVRAMPointer = 0;
	this->wram.wPlayerMovingDirection = PlayerDirectionBitmap::Null;
	this->wram.wPlayerLastStopDirection = 0;
	this->wram.wPlayerDirection = PlayerDirectionBitmap::Null;
	this->wram.wTilesetBank = 0;
	this->wram.wTilesetBlocksPtr = 0;
	this->wram.wTilesetGfxPtr = 0;
	this->wram.wTilesetCollisionPtr = 0;
	this->wram.wTilesetTalkingOverTiles.fill_bytes(0);
	this->wram.wGrassTile = 0;
	this->wram.wNumBoxItems = 0;
	this->wram.wBoxItems.fill_bytes(0);
	this->wram.wBoxItemsTerminator = 0;
	this->wram.wCurrentBoxNum = 0;
	this->wram.wNumHoFTeams = 0;
	this->wram.wUnusedD5A3 = 0;
	this->wram.wPlayerCoins = 0;
	this->wram.wMissableObjectFlags.fill_bytes(0);
	this->wram.wd5cd = 0;
	this->wram.wMissableObjectList.fill_bytes(0);
	this->wram.wOaksLabCurScript = 0;
	this->wram.wGameProgressFlags.fill_bytes(0);
	this->wram.wPalletTownCurScript = 0;
	this->wram.wBluesHouseCurScript = 0;
	this->wram.wViridianCityCurScript = 0;
	this->wram.wPewterCityCurScript = 0;
	this->wram.wRoute3CurScript = 0;
	this->wram.wRoute4CurScript = 0;
	this->wram.wViridianGymCurScript = 0;
	this->wram.wPewterGymCurScript = 0;
	this->wram.wCeruleanGymCurScript = 0;
	this->wram.wVermilionGymCurScript = 0;
	this->wram.wCeladonGymCurScript = 0;
	this->wram.wRoute6CurScript = 0;
	this->wram.wRoute8CurScript = 0;
	this->wram.wRoute24CurScript = 0;
	this->wram.wRoute25CurScript = 0;
	this->wram.wRoute9CurScript = 0;
	this->wram.wRoute10CurScript = 0;
	this->wram.wMtMoon1CurScript = 0;
	this->wram.wMtMoon3CurScript = 0;
	this->wram.wSSAnne8CurScript = 0;
	this->wram.wSSAnne9CurScript = 0;
	this->wram.wRoute22CurScript = 0;
	this->wram.wRedsHouse2CurScript = 0;
	this->wram.wViridianMarketCurScript = 0;
	this->wram.wRoute22GateCurScript = 0;
	this->wram.wCeruleanCityCurScript = 0;
	this->wram.wSSAnne5CurScript = 0;
	this->wram.wViridianForestCurScript = 0;
	this->wram.wMuseum1fCurScript = 0;
	this->wram.wRoute13CurScript = 0;
	this->wram.wRoute14CurScript = 0;
	this->wram.wRoute17CurScript = 0;
	this->wram.wRoute19CurScript = 0;
	this->wram.wRoute21CurScript = 0;
	this->wram.wSafariZoneEntranceCurScript = 0;
	this->wram.wRockTunnel2CurScript = 0;
	this->wram.wRockTunnel1CurScript = 0;
	this->wram.wRoute11CurScript = 0;
	this->wram.wRoute12CurScript = 0;
	this->wram.wRoute15CurScript = 0;
	this->wram.wRoute16CurScript = 0;
	this->wram.wRoute18CurScript = 0;
	this->wram.wRoute20CurScript = 0;
	this->wram.wSSAnne10CurScript = 0;
	this->wram.wVermilionCityCurScript = 0;
	this->wram.wPokemonTower2CurScript = 0;
	this->wram.wPokemonTower3CurScript = 0;
	this->wram.wPokemonTower4CurScript = 0;
	this->wram.wPokemonTower5CurScript = 0;
	this->wram.wPokemonTower6CurScript = 0;
	this->wram.wPokemonTower7CurScript = 0;
	this->wram.wRocketHideout1CurScript = 0;
	this->wram.wRocketHideout2CurScript = 0;
	this->wram.wRocketHideout3CurScript = 0;
	this->wram.wRocketHideout4CurScript = 0;
	this->wram.wRoute6GateCurScript = 0;
	this->wram.wRoute8GateCurScript = 0;
	this->wram.wCinnabarIslandCurScript = 0;
	this->wram.wMansion1CurScript = 0;
	this->wram.wMansion2CurScript = 0;
	this->wram.wMansion3CurScript = 0;
	this->wram.wMansion4CurScript = 0;
	this->wram.wVictoryRoad2CurScript = 0;
	this->wram.wVictoryRoad3CurScript = 0;
	this->wram.wFightingDojoCurScript = 0;
	this->wram.wSilphCo2CurScript = 0;
	this->wram.wSilphCo3CurScript = 0;
	this->wram.wSilphCo4CurScript = 0;
	this->wram.wSilphCo5CurScript = 0;
	this->wram.wSilphCo6CurScript = 0;
	this->wram.wSilphCo7CurScript = 0;
	this->wram.wSilphCo8CurScript = 0;
	this->wram.wSilphCo9CurScript = 0;
	this->wram.wHallOfFameRoomCurScript = 0;
	this->wram.wGaryCurScript = 0;
	this->wram.wLoreleiCurScript = 0;
	this->wram.wBrunoCurScript = 0;
	this->wram.wAgathaCurScript = 0;
	this->wram.wUnknownDungeon3CurScript = 0;
	this->wram.wVictoryRoad1CurScript = 0;
	this->wram.wLanceCurScript = 0;
	this->wram.wSilphCo10CurScript = 0;
	this->wram.wSilphCo11CurScript = 0;
	this->wram.wFuchsiaGymCurScript = 0;
	this->wram.wSaffronGymCurScript = 0;
	this->wram.wCinnabarGymCurScript = 0;
	this->wram.wCeladonGameCornerCurScript = 0;
	this->wram.wRoute16GateCurScript = 0;
	this->wram.wBillsHouseCurScript = 0;
	this->wram.wRoute5GateCurScript = 0;
	this->wram.wRoute7GateCurScript = 0;
	this->wram.wPowerPlantCurScript = 0;
	this->wram.wSSAnne2CurScript = 0;
	this->wram.wSeafoamIslands4CurScript = 0;
	this->wram.wRoute23CurScript = 0;
	this->wram.wSeafoamIslands5CurScript = 0;
	this->wram.wRoute18GateCurScript = 0;
	this->wram.wObtainedHiddenItemsFlags.fill_bytes(0);
	this->wram.wObtainedHiddenCoinsFlags.fill_bytes(0);
	this->wram.wWalkBikeSurfState = 0;
	this->wram.wTownVisitedFlag = 0;
	this->wram.wSafariSteps = 0;
	this->wram.wFossilItem = 0;
	this->wram.wFossilMon = 0;
	this->wram.wEnemyMonOrTrainerClass = 0;
	this->wram.wPlayerJumpingYScreenCoordsIndex = 0;
	this->wram.wRivalStarter = 0;
	this->wram.wPlayerStarter = 0;
	this->wram.wBoulderSpriteIndex = 0;
	this->wram.wLastBlackoutMap = 0;
	this->wram.wDestinationMap = MapId::PalletTown;
	this->wram.wUnusedD71B = 0;
	this->wram.wTileInFrontOfBoulderAndBoulderCollisionResult = 0;
	this->wram.wDungeonWarpDestinationMap = 0;
	this->wram.wWhichDungeonWarp = 0;
	this->wram.wUnusedD71F = 0;
	this->wram.wd728.clear();
	this->wram.wBeatGymFlags.clear();
	this->wram.wd72c.clear();
	this->wram.wd72d_destinationMap = 0;
	this->wram.wd72d.clear();
	this->wram.wd72e.clear();
	this->wram.wd730.clear();
	this->wram.wd732.clear();
	this->wram.wFlags_D733.clear();
	this->wram.wBeatLorelei.clear();
	this->wram.wd736.clear();
	this->wram.wCompletedInGameTradeFlags.fill_bytes(0);
	this->wram.wWarpedFromWhichWarp = 0;
	this->wram.wWarpedFromWhichMap = 0;
	this->wram.wCardKeyDoorY = 0;
	this->wram.wCardKeyDoorX = 0;
	this->wram.wFirstLockTrashCanIndex = 0;
	this->wram.wSecondLockTrashCanIndex = 0;
	this->wram.wEventFlags.fill_bytes(0);
	this->wram.wGrassRate = 0;
	this->wram.wLinkEnemyTrainerName.fill_bytes(0);
	this->wram.wGrassMons.fill_bytes(0);
	this->wram.wSerialEnemyDataBlock.fill_bytes(0);
	this->wram.wEnemyPartyCount = 0;
	this->wram.wEnemyPartyMons.fill_bytes(0);
	this->wram.wEnemyPartyMonsTerminator = 0;
	this->wram.wEnemyMons.fill_bytes(0);
	this->wram.wWaterRate = 0;
	this->wram.wWaterMons = 0;
	this->wram.wEnemyMonOT.fill_bytes(0);
	this->wram.wEnemyMonNicks.fill_bytes(0);
	this->wram.wTrainerHeaderPtr = 0;
	this->wram.wUnusedDA38 = 0;
	this->wram.wOpponentAfterWrongAnswer = 0;
	this->wram.wCurMapScript = 0;
	this->wram.wPlayTimeHours = 0;
	this->wram.wPlayTimeMaxed = 0;
	this->wram.wPlayTimeMinutes = 0;
	this->wram.wPlayTimeSeconds = 0;
	this->wram.wPlayTimeFrames = 0;
	this->wram.wSafariZoneGameOver = 0;
	this->wram.wNumSafariBalls = 0;
	this->wram.wDayCareInUse = 0;
	this->wram.wDayCareMonName.fill_bytes(0);
	this->wram.wDayCareMonOT.fill_bytes(0);
	this->wram.wDayCareMon.clear();
	this->wram.wNumInBox = 0;
	this->wram.wBoxSpecies.fill_bytes(0);
	this->wram.wBoxSpeciesTerminator = 0;
	this->wram.wBoxMons.clear();
	this->wram.wBoxMonOT.fill_bytes(0);
	this->wram.wBoxMonNicks.fill_bytes(0);
}

void CppRed::initialize_player_data(){
	auto wd732 = this->wram.wd732.get_raw_value();
	auto wOptions = this->wram.wOptions.get_raw_value();
	auto wLetterPrintingDelayFlags = +this->wram.wLetterPrintingDelayFlags;

	this->mass_initialization();
	this->wram.wSpriteStateData1.fill_bytes(0);
	this->wram.wSpriteStateData2.fill_bytes(0);

	this->wram.wd732.set_raw_value(wd732);
	this->wram.wOptions.set_raw_value(wOptions);
	this->wram.wLetterPrintingDelayFlags = wLetterPrintingDelayFlags;

	if (!this->wram.wOptionsInitialized)
		this->initialize_options();

	this->wram.wPlayerName = ninten_text;
	this->wram.wRivalName = sony_text;
}

void CppRed::initialize_options(){
	this->wram.wLetterPrintingDelayFlags = 1;
	this->wram.wOptions.clear();
	this->wram.wOptions.set_text_speed(TextSpeed::Medium);
}

void CppRed::add_item_to_inventory(unsigned position, ItemId item, unsigned quantity){
	this->wram.wcf91 = (unsigned)item;
	this->wram.wItemQuantity = quantity;
	//TODO: complete me.
}
