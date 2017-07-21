#include "CppRed.h"

CppRed::CppRed(){
	this->init();
}

void CppRed::init(){
	this->set_bg_scroll(0, 0);
	this->set_window_position(0, 0);
	//TODO:
	//SB   <- 0
	//SC   <- 0
	//TMA  <- 0
	//TMA  <- 0
	//BGP  <- 0
	//OBP0 <- 0
	//OBP1 <- 0
	//LCDC <- 1 << 7 (enable display)

	this->disable_lcd();

	//rLCDC_DEFAULT EQU %11100011
	// * LCD enabled
	// * Window tile map at $9C00
	// * Window display enabled
	// * BG and window tile data at $8800
	// * BG tile map at $9800
	// * 8x8 OBJ size
	// * OBJ display enabled
	// * BG display enabled
}

/*
 *	DisableLCD::
 *		xor a
 *		ld [rIF], a
 *		ld a, [rIE]
 *		ld b, a
 *		res 0, a
 *		ld [rIE], a
 *	
 *	.wait
 *		ld a, [rLY]
 *		cp LY_VBLANK
 *		jr nz, .wait
 *	
 *		ld a, [rLCDC]
 *		and $ff ^ rLCDC_ENABLE_MASK
 *		ld [rLCDC], a
 *		ld a, b
 *		ld [rIE], a
 *		ret
 *	
 *	void CppRed::disable_lcd(){
 *		
 *	}
 */
