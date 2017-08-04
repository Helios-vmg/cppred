#pragma once

#include "CommonTypes.h"

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

enum class SpeciesId{
	None = 0,
	Rhydon = 1,
	Kangaskhan = 2,
	NidoranMale = 3,
	Clefairy = 4,
	Spearow = 5,
	Voltorb = 6,
	Nidoking = 7,
	Slowbro = 8,
	Ivysaur = 9,
	Exeggutor = 10,
	Lickitung = 11,
	Exeggcute = 12,
	Grimer = 13,
	Gengar = 14,
	NidoranFemale = 15,
	Nidoqueen = 16,
	Cubone = 17,
	Rhyhorn = 18,
	Lapras = 19,
	Arcanine = 20,
	Mew = 21,
	Gyarados = 22,
	Shellder = 23,
	Tentacool = 24,
	Gastly = 25,
	Scyther = 26,
	Staryu = 27,
	Blastoise = 28,
	Pinsir = 29,
	Tangela = 30,
	MissingNo0x1F = 31,
	MissingNo0x20 = 32,
	Growlithe = 33,
	Onix = 34,
	Fearow = 35,
	Pidgey = 36,
	Slowpoke = 37,
	Kadabra = 38,
	Graveler = 39,
	Chansey = 40,
	Machoke = 41,
	MrMime = 42,
	Hitmonlee = 43,
	Hitmonchan = 44,
	Arbok = 45,
	Parasect = 46,
	Psyduck = 47,
	Drowzee = 48,
	Golem = 49,
	MissingNo0x32 = 50,
	Magmar = 51,
	MissingNo0x34 = 52,
	Electabuzz = 53,
	Magneton = 54,
	Koffing = 55,
	MissingNo0x38 = 56,
	Mankey = 57,
	Seel = 58,
	Diglett = 59,
	Tauros = 60,
	MissingNo0x3D = 61,
	MissingNo0x3E = 62,
	MissingNo0x3F = 63,
	Farfetchd = 64,
	Venonat = 65,
	Dragonite = 66,
	MissingNo0x43 = 67,
	MissingNo0x44 = 68,
	MissingNo0x45 = 69,
	Doduo = 70,
	Poliwag = 71,
	Jynx = 72,
	Moltres = 73,
	Articuno = 74,
	Zapdos = 75,
	Ditto = 76,
	Meowth = 77,
	Krabby = 78,
	MissingNo0x4F = 79,
	MissingNo0x50 = 80,
	MissingNo0x51 = 81,
	Vulpix = 82,
	Ninetales = 83,
	Pikachu = 84,
	Raichu = 85,
	MissingNo0x56 = 86,
	MissingNo0x57 = 87,
	Dratini = 88,
	Dragonair = 89,
	Kabuto = 90,
	Kabutops = 91,
	Horsea = 92,
	Seadra = 93,
	MissingNo0x5E = 94,
	MissingNo0x5F = 95,
	Sandshrew = 96,
	Sandslash = 97,
	Omanyte = 98,
	Omastar = 99,
	Jigglypuff = 100,
	Wigglytuff = 101,
	Eevee = 102,
	Flareon = 103,
	Jolteon = 104,
	Vaporeon = 105,
	Machop = 106,
	Zubat = 107,
	Ekans = 108,
	Paras = 109,
	Poliwhirl = 110,
	Poliwrath = 111,
	Weedle = 112,
	Kakuna = 113,
	Beedrill = 114,
	MissingNo0x73 = 115,
	Dodrio = 116,
	Primeape = 117,
	Dugtrio = 118,
	Venomoth = 119,
	Dewgong = 120,
	MissingNo0x79 = 121,
	MissingNo0x7A = 122,
	Caterpie = 123,
	Metapod = 124,
	Butterfree = 125,
	Machamp = 126,
	MissingNo0x7F = 127,
	Golduck = 128,
	Hypno = 129,
	Golbat = 130,
	Mewtwo = 131,
	Snorlax = 132,
	Magikarp = 133,
	MissingNo0x86 = 134,
	MissingNo0x87 = 135,
	Muk = 136,
	Kingler = 138,
	MissingNo0x8A = 138,
	Cloyster = 139,
	MissingNo0x8C = 140,
	Electrode = 141,
	Clefable = 142,
	Weezing = 143,
	Persian = 144,
	Marowak = 145,
	MissingNo0x92 = 146,
	Haunter = 147,
	Abra = 148,
	Alakazam = 149,
	Pidgeotto = 150,
	Pidgeot = 151,
	Starmie = 152,
	Bulbasaur = 153,
	Venusaur = 154,
	Tentacruel = 155,
	MissingNo0x9C = 156,
	Goldeen = 157,
	Seaking = 158,
	MissingNo0x9F = 159,
	MissingNo0xA0 = 160,
	MissingNo0xA1 = 161,
	MissingNo0xA2 = 162,
	Ponyta = 163,
	Rapidash = 164,
	Rattata = 165,
	Raticate = 166,
	Nidorino = 167,
	Nidorina = 168,
	Geodude = 169,
	Porygon = 170,
	Aerodactyl = 171,
	MissingNo0xAC = 172,
	Magnemite = 173,
	MissingNo0xAE = 174,
	MissingNo0xAF = 175,
	Charmander = 176,
	Squirtle = 177,
	Charmeleon = 178,
	Wartortle = 179,
	Charizard = 180,
	MissingNo0xB5 = 181,
	FossilKabutops = 182,
	FossilAerodactyl = 183,
	MonGhost = 184,
	Oddish = 185,
	Gloom = 186,
	Vileplume = 187,
	Bellsprout = 188,
	Weepinbell = 189,
	Victreebel = 190,
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
	Stop = 0xFF,
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

const unsigned bg_map0 = 0x9800;
const unsigned bg_map1 = 0x9C00;

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

const unsigned vChars0 = 0x8000;
const unsigned vChars1 = 0x8800;
const unsigned vSprites = vChars0;
const unsigned vFont = vChars1;

enum class MenuType{
	YesNo,
	NorthWest,
	SouthEast,
	WideYesNo,
	NorthEast,
	TradeCancel,
	HealCancel,
	NoYes,
};

enum class TextBoxId{
	MessageBox = 0x01,
	FieldMoveMonMenu = 0x04,
	JpMochimonoMenuTemplate = 0x05,
	UseTossMenuTemplate = 0x06,
	JpSaveMessageMenuTemplate = 0x08,
	JpSpeedOptionsMenuTemplate = 0x09,
	BattleMenuTemplate = 0x0b,
	SwitchStatsCancelMenuTemplate = 0x0c,
	ListMenuBox = 0x0d,
	BuySellQuitMenuTemplate = 0x0e,
	MoneyBoxTemplate = 0x0f,
	MonSpritePopup = 0x11,
	JpAhMenuTemplate = 0x12,
	MoneyBox = 0x13,
	TwoOptionMenu = 0x14,
	BuySellQuitMenu = 0x15,
	JpPokedexMenuTemplate = 0x1a,
	SafariBattleMenuTemplate = 0x1b,
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
	Right = 1 << direction_r_bit,
	Left  = 1 << direction_l_bit,
	Down  = 1 << direction_d_bit,
	Up    = 1 << direction_u_bit,
};

enum class PlayerDirectionBitmap{
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
