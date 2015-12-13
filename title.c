#include "title.h"
#include "main.h"
#include <gb/gb.h>

void show_title() {
	disable_interrupts();
	DISPLAY_OFF;
	HIDE_SPRITES;
	HIDE_WIN;
			
	// This is exceedingly lazy/poor use of ROM space... It's one bank, and this is an example program. Deal with it.
	set_bkg_data(0U, 128U, title_tiles);
	set_bkg_tiles(0U, 0U, 20U, 20U, title_screens);
	SHOW_BKG;
	
	DISPLAY_ON;
	enable_interrupts();

	oldBtns = btns;
	while (!(!(oldBtns & J_START) && btns & J_START)) {
		oldBtns = btns;
		btns = joypad();
		wait_vbl_done();
	}
	return;
}

void show_game_over() {
	disable_interrupts();
	DISPLAY_OFF;
	HIDE_SPRITES;
	HIDE_WIN;
			
	// This is exceedingly lazy/poor use of ROM space... It's one bank, and this is an example program. Deal with it.
	set_bkg_data(0U, 128U, title_tiles);
	set_bkg_tiles(0U, 0U, 20U, 20U, title_screens+(20U*16U));
	SHOW_BKG;
	
	DISPLAY_ON;
	enable_interrupts();

	oldBtns = btns;
	while (!(!(oldBtns & J_START) && btns & J_START)) {
		oldBtns = btns;
		btns = joypad();
		wait_vbl_done();
	}
}