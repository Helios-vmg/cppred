#pragma once

#include "CommonTypes.h"
#include "utility.h"
#include "../CodeGeneration/output/pokemon_enums.h"

#define RED 'RED'
#define BLUE 'BLUE'
#define GREEN 'GREE'
#define POKEMON_VERSION RED
#include <string>
//#include <map>

enum class SerialConnectionStatus{
	UsingExternalClock = 1,
	UsingInternalClock = 2,
	ConnectionNotEstablished = 0xFF,
};

enum class Predef{
	DrawPlayerHUDAndHPBar = 0,
	CopyUncompressedPicToTilemap = 1,
	AnimateSendingOutMon = 2,
	ScaleSpriteByTwo = 3,
	LoadMonBackPic = 4,
	CopyDownscaledMonTiles = 5,
	LoadMissableObjects = 6,
	HealParty = 7,
	MoveAnimation = 8,
	DivideBCDPredef = 9,
	DivideBCDPredef2 = 10,
	AddBCDPredef = 11,
	SubBCDPredef = 12,
	DivideBCDPredef3 = 13,
	DivideBCDPredef4 = 14,
	InitPlayerData = 15,
	FlagActionPredef = 16,
	HideObject = 17,
	IsObjectHidden = 18,
	ApplyOutOfBattlePoisonDamage = 19,
	AnyPartyAlive = 20,
	ShowObject = 21,
	ShowObject2 = 22,
	ReplaceTileBlock = 23,
	InitPlayerData2 = 24,
	LoadTilesetHeader = 25,
	LearnMoveFromLevelUp = 26,
	LearnMove = 27,
	GetQuantityOfItemInBag = 28,
	CheckForHiddenObjectOrBookshelfOrCardKeyDoor = 29, //home bank
	GiveItem = 30, //home bank
	ChangeBGPalColor0_4Frames = 31,
	FindPathToPlayer = 32,
	PredefShakeScreenVertically = 33,
	CalcPositionOfPlayerRelativeToNPC = 34,
	ConvertNPCMovementDirectionsToJoypadMasks = 35,
	PredefShakeScreenHorizontally = 36,
	UpdateHPBar = 37,
	HPBarLength = 38,
	Diploma_TextBoxBorder = 39,
	DoubleOrHalveSelectedStats = 40,
	ShowPokedexMenu = 41,
	EvolutionAfterBattle = 42,
	SaveSAVtoSRAM0 = 43,
	InitOpponent = 44,
	CableClub_Run = 45,
	DrawBadges = 46,
	ExternalClockTradeAnim = 47,
	BattleTransition = 48,
	CopyTileIDsFromList = 49,
	PlayIntro = 50,
	GetMoveSoundB = 51,
	FlashScreen = 52,
	GetTileAndCoordsInFrontOfPlayer = 53,
	StatusScreen = 54,
	StatusScreen2 = 55,
	InternalClockTradeAnim = 56,
	TrainerEngage = 57,
	IndexToPokedex = 58,
	DisplayPicCenteredOrUpperRight = 59,
	UsedCut = 60,
	ShowPokedexData = 61,
	WriteMonMoves = 62,
	SaveSAV = 63,
	LoadSGB = 64,
	MarkTownVisitedAndLoadMissableObjects = 65,
	SetPartyMonTypes = 66,
	CanLearnTM = 67,
	TMToMove = 68,
	_RunPaletteCommand = 69,
	StarterDex = 70,
	_AddPartyMon = 71,
	UpdateHPBar2 = 72,
	DrawEnemyHUDAndHPBar = 73,
	LoadTownMap_Nest = 74,
	PrintMonType = 75,
	EmotionBubble = 76,
	EmptyFunc3 = 77, //return immediately
	AskName = 78,
	PewterGuys = 79,
	SaveSAVtoSRAM2 = 80,
	LoadSAV2 = 81,
	LoadSAV = 82,
	SaveSAVtoSRAM1 = 83,
	DoInGameTradeDialogue = 84,
	HallOfFamePC = 85,
	DisplayDexRating = 86,
	_LeaveMapAnim = 87, //wrong bank
	EnterMapAnim = 88, //wrong bank
	GetTileTwoStepsInFrontOfPlayer = 89,
	CheckForCollisionWhenPushingBoulder = 90,
	PrintStrengthTxt = 91,
	PickUpItem = 92,
	PrintMoveType = 93,
	LoadMovePPs = 94,
	DrawHP = 95,
	DrawHP2 = 96,
	DisplayElevatorFloorMenu = 97,
	OaksAideScript = 98,
	COUNT,
};

enum class PaletteCommand{
	//SET_PAL_BATTLE_BLACK         EQU $00
	SetPaletteBattleBlack = 0x00,
	//SET_PAL_BATTLE               EQU $01
	SetPaletteBattle = 0x01,
	//SET_PAL_TOWN_MAP             EQU $02
	SetPaletteTownMap = 0x02,
	//SET_PAL_STATUS_SCREEN        EQU $03
	SetPaletteStatusScreen = 0x03,
	//SET_PAL_POKEDEX              EQU $04
	SetPalettePokedex = 0x04,
	//SET_PAL_SLOTS                EQU $05
	SetPaletteSlots = 0x05,
	//SET_PAL_TITLE_SCREEN         EQU $06
	SetPaletteTitleScreen = 0x06,
	//SET_PAL_NIDORINO_INTRO       EQU $07
	SetPaletteNidorinoIntro = 0x07,
	//SET_PAL_GENERIC              EQU $08
	SetPaletteGeneric = 0x08,
	//SET_PAL_OVERWORLD            EQU $09
	SetPaletteOverworld = 0x09,
	//SET_PAL_PARTY_MENU           EQU $0A
	SetPalettePartyMenu = 0x0A,
	//SET_PAL_POKEMON_WHOLE_SCREEN EQU $0B
	SetPalettePokemonWholeScreen = 0x0B,
	//SET_PAL_GAME_FREAK_INTRO     EQU $0C
	SetPaletteGameFreakIntro = 0x0C,
	//SET_PAL_TRAINER_CARD         EQU $0D
	SetPaletteTrainerCard = 0x0D,
	//UPDATE_PARTY_MENU_BLK_PACKET EQU $FC
	UpdatePartyMenuBlkPacket = 0xFC,
	Default = 0xFF,
};

enum class Sound{
	None = 0,
	//AUDIO_1
	Music_PalletTown,
	Music_Pokecenter,
	Music_Gym,
	Music_Cities1,
	Music_Cities2,
	Music_Celadon,
	Music_Cinnabar,
	Music_Vermilion,
	Music_Lavender,
	Music_SSAnne,
	Music_MeetProfOak,
	Music_MeetRival,
	Music_MuseumGuy,
	Music_SafariZone,
	Music_PkmnHealed,
	Music_Routes1,
	Music_Routes2,
	Music_Routes3,
	Music_Routes4,
	Music_IndigoPlateau,

	//AUDIO_2
	Music_GymLeaderBattle,
	Music_TrainerBattle,
	Music_WildBattle,
	Music_FinalBattle,
	Music_DefeatedTrainer,
	Music_DefeatedWildMon,
	Music_DefeatedGymLeader,

	//AUDIO_3
	Music_TitleScreen,
	Music_Credits,
	Music_HallOfFame,
	Music_OaksLab,
	Music_JigglypuffSong,
	Music_BikeRiding,
	Music_Surfing,
	Music_GameCorner,
	Music_IntroBattle,
	Music_Dungeon1,
	Music_Dungeon2,
	Music_Dungeon3,
	Music_CinnabarMansion,
	Music_PokemonTower,
	Music_SilphCo,
	Music_MeetEvilTrainer,
	Music_MeetFemaleTrainer,
	Music_MeetMaleTrainer,

	//AUDIO_1 AUDIO_2 AUDIO_3
	SFX_Snare1_1,
	SFX_Snare2_1,
	SFX_Snare3_1,
	SFX_Snare4_1,
	SFX_Snare5_1,
	SFX_Triangle1_1,
	SFX_Triangle2_1,
	SFX_Snare6_1,
	SFX_Snare7_1,
	SFX_Snare8_1,
	SFX_Snare9_1,
	SFX_Cymbal1_1,
	SFX_Cymbal2_1,
	SFX_Cymbal3_1,
	SFX_Muted_Snare1_1,
	SFX_Triangle3_1,
	SFX_Muted_Snare2_1,
	SFX_Muted_Snare3_1,
	SFX_Muted_Snare4_1,
	SFX_Cry00_1,
	SFX_Cry01_1,
	SFX_Cry02_1,
	SFX_Cry03_1,
	SFX_Cry04_1,
	SFX_Cry05_1,
	SFX_Cry06_1,
	SFX_Cry07_1,
	SFX_Cry08_1,
	SFX_Cry09_1,
	SFX_Cry0A_1,
	SFX_Cry0B_1,
	SFX_Cry0C_1,
	SFX_Cry0D_1,
	SFX_Cry0E_1,
	SFX_Cry0F_1,
	SFX_Cry10_1,
	SFX_Cry11_1,
	SFX_Cry12_1,
	SFX_Cry13_1,
	SFX_Cry14_1,
	SFX_Cry15_1,
	SFX_Cry16_1,
	SFX_Cry17_1,
	SFX_Cry18_1,
	SFX_Cry19_1,
	SFX_Cry1A_1,
	SFX_Cry1B_1,
	SFX_Cry1C_1,
	SFX_Cry1D_1,
	SFX_Cry1E_1,
	SFX_Cry1F_1,
	SFX_Cry20_1,
	SFX_Cry21_1,
	SFX_Cry22_1,
	SFX_Cry23_1,
	SFX_Cry24_1,
	SFX_Cry25_1,

	SFX_Get_Item2_1,
	SFX_Tink_1,
	SFX_Heal_HP_1,
	SFX_Heal_Ailment_1,
	SFX_Start_Menu_1,
	SFX_Press_AB_1,

	//AUDIO_1 AUDIO_3
	SFX_Get_Item1_1,

	SFX_Pokedex_Rating_1,
	SFX_Get_Key_Item_1,
	SFX_Poisoned_1,
	SFX_Trade_Machine_1,
	SFX_Turn_On_PC_1,
	SFX_Turn_Off_PC_1,
	SFX_Enter_PC_1,
	SFX_Shrink_1,
	SFX_Switch_1,
	SFX_Healing_Machine_1,
	SFX_Teleport_Exit1_1,
	SFX_Teleport_Enter1_1,
	SFX_Teleport_Exit2_1,
	SFX_Ledge_1,
	SFX_Teleport_Enter2_1,
	SFX_Fly_1,
	SFX_Denied_1,
	SFX_Arrow_Tiles_1,
	SFX_Push_Boulder_1,
	SFX_SS_Anne_Horn_1,
	SFX_Withdraw_Deposit_1,
	SFX_Cut_1,
	SFX_Go_Inside_1,
	SFX_Swap_1,
	SFX_59_1, //unused, sounds similar to SFX_SLOTS_STOP_WHEEL
	SFX_Purchase_1,
	SFX_Collision_1,
	SFX_Go_Outside_1,
	SFX_Save_1,

	//AUDIO_1
	SFX_Pokeflute,
	SFX_Safari_Zone_PA,

	//AUDIO_2
	SFX_Level_Up,

	SFX_Ball_Toss,
	SFX_Ball_Poof,
	SFX_Faint_Thud,
	SFX_Run,
	SFX_Dex_Page_Added,
	SFX_Caught_Mon,
	SFX_Peck,
	SFX_Faint_Fall,
	SFX_Battle_09,
	SFX_Pound,
	SFX_Battle_0B,
	SFX_Battle_0C,
	SFX_Battle_0D,
	SFX_Battle_0E,
	SFX_Battle_0F,
	SFX_Damage,
	SFX_Not_Very_Effective,
	SFX_Battle_12,
	SFX_Battle_13,
	SFX_Battle_14,
	SFX_Vine_Whip,
	SFX_Battle_16, //unused?
	SFX_Battle_17,
	SFX_Battle_18,
	SFX_Battle_19,
	SFX_Super_Effective,
	SFX_Battle_1B,
	SFX_Battle_1C,
	SFX_Doubleslap,
	SFX_Battle_1E,
	SFX_Horn_Drill,
	SFX_Battle_20,
	SFX_Battle_21,
	SFX_Battle_22,
	SFX_Battle_23,
	SFX_Battle_24,
	SFX_Battle_25,
	SFX_Battle_26,
	SFX_Battle_27,
	SFX_Battle_28,
	SFX_Battle_29,
	SFX_Battle_2A,
	SFX_Battle_2B,
	SFX_Battle_2C,
	SFX_Psybeam,
	SFX_Battle_2E,
	SFX_Battle_2F,
	SFX_Psychic_M,
	SFX_Battle_31,
	SFX_Battle_32,
	SFX_Battle_33,
	SFX_Battle_34,
	SFX_Battle_35,
	SFX_Battle_36,
	SFX_Silph_Scope,

	//AUDIO_3
	SFX_Intro_Lunge,
	SFX_Intro_Hip,
	SFX_Intro_Hop,
	SFX_Intro_Raise,
	SFX_Intro_Crash,
	SFX_Intro_Whoosh,
	SFX_Slots_Stop_Wheel,
	SFX_Slots_Reward,
	SFX_Slots_New_Spin,
	SFX_Shooting_Star,
	Stop = 0xFF,
};

enum class ItemId{
	None = 0,
	MasterBall = 0x01,
	UltraBall = 0x02,
	GreatBall = 0x03,
	PokeBall = 0x04,
	TownMap = 0x05,
	Bicycle = 0x06,
	Surfboard = 0x07,
	SafariBall = 0x08,
	Pokedex = 0x09,
	MoonStone = 0x0A,
	Antidote = 0x0B,
	BurnHeal = 0x0C,
	IceHeal = 0x0D,
	Awakening = 0x0E,
	ParlyzHeal = 0x0F,
	FullRestore = 0x10,
	MaxPotion = 0x11,
	HyperPotion = 0x12,
	SuperPotion = 0x13,
	Potion = 0x14,
	Boulderbadge = 0x15,
	Cascadebadge = 0x16,
	SafariBait = 0x15,
	SafariRock = 0x16,
	Thunderbadge = 0x17,
	Rainbowbadge = 0x18,
	Soulbadge = 0x19,
	Marshbadge = 0x1A,
	Volcanobadge = 0x1B,
	Earthbadge = 0x1C,
	EscapeRope = 0x1D,
	Repel = 0x1E,
	OldAmber = 0x1F,
	FireStone = 0x20,
	ThunderStone = 0x21,
	WaterStone = 0x22,
	HpUp = 0x23,
	Protein = 0x24,
	Iron = 0x25,
	Carbos = 0x26,
	Calcium = 0x27,
	RareCandy = 0x28,
	DomeFossil = 0x29,
	HelixFossil = 0x2A,
	SecretKey = 0x2B,
	UnusedItem = 0x2C,
	BikeVoucher = 0x2D,
	XAccuracy = 0x2E,
	LeafStone = 0x2F,
	CardKey = 0x30,
	Nugget = 0x31,
	PpUp2 = 0x32,
	PokeDoll = 0x33,
	FullHeal = 0x34,
	Revive = 0x35,
	MaxRevive = 0x36,
	GuardSpec = 0x37,
	SuperRepel = 0x38,
	MaxRepel = 0x39,
	DireHit = 0x3A,
	Coin = 0x3B,
	FreshWater = 0x3C,
	SodaPop = 0x3D,
	Lemonade = 0x3E,
	SSTicket = 0x3F,
	GoldTeeth = 0x40,
	XAttack = 0x41,
	XDefend = 0x42,
	XSpeed = 0x43,
	XSpecial = 0x44,
	CoinCase = 0x45,
	OaksParcel = 0x46,
	Itemfinder = 0x47,
	SilphScope = 0x48,
	PokeFlute = 0x49,
	LiftKey = 0x4A,
	ExpAll = 0x4B,
	OldRod = 0x4C,
	GoodRod = 0x4D,
	SuperRod = 0x4E,
	PpUp = 0x4F,
	Ether = 0x50,
	MaxEther = 0x51,
	Elixer = 0x52,
	MaxElixer = 0x53,
	FloorB2f = 0x54,
	FloorB1f = 0x55,
	Floor1f = 0x56,
	Floor2f = 0x57,
	Floor3f = 0x58,
	Floor4f = 0x59,
	Floor5f = 0x5A,
	Floor6f = 0x5B,
	Floor7f = 0x5C,
	Floor8f = 0x5D,
	Floor9f = 0x5E,
	Floor10f = 0x5F,
	Floor11f = 0x60,
	FloorB4f = 0x61,
	Hm01 = 0xC4,
	Hm02 = 0xC5,
	Hm03 = 0xC6,
	Hm04 = 0xC7,
	Hm05 = 0xC8,
	Tm01 = 0xC9,
	Tm02 = 0xCA,
	Tm03 = 0xCB,
	Tm04 = 0xCC,
	Tm05 = 0xCD,
	Tm06 = 0xCE,
	Tm07 = 0xCF,
	Tm08 = 0xD0,
	Tm09 = 0xD1,
	Tm10 = 0xD2,
	Tm11 = 0xD3,
	Tm12 = 0xD4,
	Tm13 = 0xD5,
	Tm14 = 0xD6,
	Tm15 = 0xD7,
	Tm16 = 0xD8,
	Tm17 = 0xD9,
	Tm18 = 0xDA,
	Tm19 = 0xDB,
	Tm20 = 0xDC,
	Tm21 = 0xDD,
	Tm22 = 0xDE,
	Tm23 = 0xDF,
	Tm24 = 0xE0,
	Tm25 = 0xE1,
	Tm26 = 0xE2,
	Tm27 = 0xE3,
	Tm28 = 0xE4,
	Tm29 = 0xE5,
	Tm30 = 0xE6,
	Tm31 = 0xE7,
	Tm32 = 0xE8,
	Tm33 = 0xE9,
	Tm34 = 0xEA,
	Tm35 = 0xEB,
	Tm36 = 0xEC,
	Tm37 = 0xED,
	Tm38 = 0xEE,
	Tm39 = 0xEF,
	Tm40 = 0xF0,
	Tm41 = 0xF1,
	Tm42 = 0xF2,
	Tm43 = 0xF3,
	Tm44 = 0xF4,
	Tm45 = 0xF5,
	Tm46 = 0xF6,
	Tm47 = 0xF7,
	Tm48 = 0xF8,
	Tm49 = 0xF9,
	Tm50 = 0xFA,
};

enum class AudioBank{
	Bank1,
	Bank2,
	Bank3,
};

//extern const std::map<Sound, AudioBank> 

const unsigned tilemap_width = 20;
const unsigned tilemap_height = 18;

//const unsigned oam_location = 0xC300;
//const unsigned oam_size = 4 * 40;
const unsigned hram_location = 0xFF80;
const unsigned hram_size = 0xFFFF - 0xFF80;
const unsigned wram_location = 0xC000;
const unsigned wram_size = 0x2000;

// * LCD enabled
// * Window tile map at $9C00
// * Window display enabled
// * BG and window tile data at $8800
// * BG tile map at $9800
// * 8x8 OBJ size
// * OBJ display enabled
// * BG display enabled
const byte_t lcdc_default = 0xE3;

extern const std::string ninten_text;
extern const std::string sony_text;

const unsigned input_a      = 1 << 0;
const unsigned input_b      = 1 << 1;
const unsigned input_select = 1 << 2;
const unsigned input_start  = 1 << 3;
const unsigned input_right  = 1 << 4;
const unsigned input_left   = 1 << 5;
const unsigned input_up     = 1 << 6;
const unsigned input_down   = 1 << 7;

const unsigned bag_capacity = 20;
const unsigned pc_capacity = 50;

//Width and height of a mon or character picture.
const unsigned pic_size = 7;
//Size in bytes of a single 8x8 tile.
const unsigned tile_byte_size = 16;

const unsigned vChars0 = 0x8000;
const unsigned vChars1 = 0x8800;
const unsigned vChars2 = 0x9000;
const unsigned vFrontPic = vChars2;
const unsigned vSprites = vChars0;
const unsigned vFont = vChars1;
const unsigned vTitleLogo = vChars1;
const unsigned vTitleLogo2 = vFrontPic + pic_size * pic_size * tile_byte_size;
const unsigned vBGMap0 = 0x9800;
const unsigned vBGMap1 = 0x9C00;

enum class TwoOptionMenuType{
	YesNo       = 0,
	NorthWest   = 1,
	SouthEast   = 2,
	WideYesNo   = 3,
	NorthEast   = 4,
	TradeCancel = 5,
	HealCancel  = 6,
	NoYes       = 7,
};

enum class TextBoxId{
	MessageBox                    = 0x01,
	Invalid02                     = 0x02,
	Unknown03                     = 0x03,
	FieldMoveMonMenu              = 0x04,
	JpMochimonoMenuTemplate       = 0x05,
	UseTossMenuTemplate           = 0x06,
	Unknown07                     = 0x07,
	JpSaveMessageMenuTemplate     = 0x08,
	JpSpeedOptionsMenuTemplate    = 0x09,
	Invalid0A                     = 0x0A,
	BattleMenuTemplate            = 0x0B,
	SwitchStatsCancelMenuTemplate = 0x0C,
	ListMenuBox                   = 0x0D,
	BuySellQuitMenuTemplate       = 0x0E,
	MoneyBoxTemplate              = 0x0F,
	Unknown10                     = 0x10,
	MonSpritePopup                = 0x11,
	JpAhMenuTemplate              = 0x12,
	MoneyBox                      = 0x13,
	TwoOptionMenu                 = 0x14,
	BuySellQuitMenu               = 0x15,
	Invalid16                     = 0x16,
	Invalid17                     = 0x17,
	Invalid18                     = 0x18,
	Invalid19                     = 0x19,
	JpPokedexMenuTemplate         = 0x1A,
	SafariBattleMenuTemplate      = 0x1B,
};

enum class BattleStyle{
	Shift = 0,
	Set = 1,
};

enum class TextSpeed{
	Fast     = 1,
	Medium   = 3,
	Slow     = 5,
	Invalid0 = 0,
	Invalid2 = 2,
	Invalid4 = 4,
	Invalid6 = 6,
	Invalid7 = 7,
};

enum class LinkState{
	//not using link
	None = 0,
	//in a cable club room(Colosseum or Trade Centre)
	InCableClub = 1,
	//pre - trade selection screen initialisation
	StartTrade = 2,
	//pre - battle initialisation
	StartBattle = 3,
	//in a link battle
	Battling = 4,
	//reset game(unused)
	Reset = 5,
	//in a link trade
	Trading = 0x32,
};

enum class SaveFileStatus{
	NoSave = 1,
	SaveExists = 2,
};

const unsigned direction_r_bit = 0;
const unsigned direction_l_bit = 1;
const unsigned direction_d_bit = 2;
const unsigned direction_u_bit = 3;

enum class PlayerDirection{
	Right = direction_r_bit,
	Left  = direction_l_bit,
	Down  = direction_d_bit,
	Up    = direction_u_bit,
};

enum class DirectionBitmap{
	Null  = 0,
	Right = 1 << direction_r_bit,
	Left  = 1 << direction_l_bit,
	Down  = 1 << direction_d_bit,
	Up    = 1 << direction_u_bit,
};

enum class PlayerDirectionBitmap{
	Null  = (int)DirectionBitmap::Null,
	Right = (int)DirectionBitmap::Right,
	Left  = (int)DirectionBitmap::Left,
	Down  = (int)DirectionBitmap::Down,
	Up    = (int)DirectionBitmap::Up,
};

enum class SpriteFacingDirection{
	Down  = 4 * 0,
	Up    = 4 * 1,
	Left  = 4 * 2,
	Right = 4 * 3,
};

enum class NpcMovementDirection{
	Down  = 4 * 0,
	Up    = 4 * 1,
	Left  = 4 * 2,
	Right = 4 * 3,
};

enum class MovementStatus{
	Uninitialized = 0,
	Ready = 1,
	Delayed = 2,
	Moving = 3,
};

enum class FlagAction{
	Reset,
	Set,
	Test,
};

const unsigned party_length = 6;
const unsigned character_name_length = 11;
const unsigned trainer_name_length = 13;
const unsigned item_name_length = 13;
const unsigned move_name_length = 11; //TODO: Confirm this value.
const unsigned species_count = 151;
const unsigned player_inventory_size = 40;
const unsigned pc_inventory_size = 100;
const unsigned mons_per_box = 20;
const unsigned hall_of_fame_mon = 16;
const unsigned hall_of_fame_team = hall_of_fame_mon * party_length;
const unsigned hall_of_fame_capacity = 50;

enum class PokemonTypeId{
	Normal = 0,
	Fighting = 1,
	Flying = 2,
	Poison = 3,
	Ground = 4,
	Rock = 5,
	Bug = 7,
	Ghost = 8,
	Fire = 20,
	Water = 21,
	Grass = 22,
	Electric = 23,
	Psychic = 24,
	Ice = 25,
	Dragon = 26,
};

#include "../CodeGeneration/output/move_enums.h"

enum class MenuExitMethod{
	SelectedAnItem       = 1,
	MenuCancelled        = 2,
	SelectedFirstOption  = 1,
	SelectedSecondOption = 2,
};

enum class PartyMenuHpColor{
	Green = 0,
	Yellow = 1,
	Red = 2,
};

enum class PokemonOverworldSprite{
	Mon       = 0, //Generic mon
	Ball      = 1, //Pokeball
	Helix     = 2, //Spiral shell
	Fairy     = 3, //Cleraify-like
	Bird      = 4, //Bird
	Water     = 5, //Seal
	Bug       = 6, //Bug
	Grass     = 7, //Flower with eyes
	Snake     = 8, //Snake
	Quadruped = 9, //Bovine
	Invalid   = 10,
	Invalid11 = 11,
	Invalid12 = 12,
	Invalid13 = 13,
	Invalid14 = 14,
	Invalid15 = 15,
	Count,
};

const unsigned tiles_per_pokemon_ow_sprite = 4;
const unsigned gb_max_sprite_count = 40;

enum class Placing{
	Centered,
	TopLeft,
};

enum class NamingScreenType{
	PlayerName = 0,
	RivalName = 1,
	PokemonNickname = 2,
};

const unsigned oamflag_canbemasked = bits_from_u32<0x00000010>::value;
const unsigned oamflag_vflipped    = bits_from_u32<0x00100000>::value;
const unsigned oam_hflip           = bits_from_u32<0x00100000>::value;
const unsigned oam_vflip           = bits_from_u32<0x01000000>::value;
