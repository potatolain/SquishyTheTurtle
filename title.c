#include "title.h"
#include "main.h"
#include "sram.h"
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
	
	waitpadup();
	oldBtns = btns;
	btns = joypad();
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
	set_bkg_tiles(0U, 0U, 20U, 20U, title_screens+(20U*18U));
	SHOW_BKG;
	
	DISPLAY_ON;
	enable_interrupts();

	waitpadup();
	oldBtns = btns;
	btns = joypad();
	while (!(!(oldBtns & J_START) && btns & J_START)) {
		oldBtns = btns;
		btns = joypad();
		wait_vbl_done();
	}
}

void show_win_screen() {
	disable_interrupts();
	DISPLAY_OFF;
	HIDE_SPRITES;
	HIDE_WIN;
			
	// This is exceedingly lazy/poor use of ROM space... It's one bank, and this is an example program. Deal with it.
	set_bkg_data(0U, 128U, title_tiles);
	
	// Clean it up...
	for (i = 0; i < 20; i++)
		buffer[i] = BLANK_TILE;
	for (i = 0; i < 16U; i++)
		set_bkg_tiles(0U, i, 20U, 1U, buffer);
	
	// Gross...
	buffer[5] = ('Y'   -'A'+ALPHA_START);
	buffer[6] = ('O'   -'A'+ALPHA_START);
	buffer[7] = ('U'   -'A'+ALPHA_START);
	// buffer[8] = BLANK_TILE;
	buffer[9] = ('W'   -'A'+ALPHA_START);
	buffer[10] = ('I'   -'A'+ALPHA_START);
	buffer[11] = ('N'   -'A'+ALPHA_START);
	buffer[12] = ALPHA_START+26; // !
	
	set_bkg_tiles(0U, 4U, 20U, 1U, buffer);
	
	for (i = 0; i < 20; i++)
		buffer[i] = BLANK_TILE;
	
	// Gross... [2]
	buffer[5] = ('T'   -'A'+ALPHA_START);
	buffer[6] = ('H'   -'A'+ALPHA_START);
	buffer[7] = ('A'   -'A'+ALPHA_START);
	buffer[8] = ('N'   -'A'+ALPHA_START);
	buffer[9] = ('K'   -'A'+ALPHA_START);
	buffer[10] = ('S'   -'A'+ALPHA_START);
	// buffer[11] = BLANK_TILE;
	buffer[12] = ('F'   -'A'+ALPHA_START);
	buffer[13] = ('O'   -'A'+ALPHA_START);
	buffer[14] = ('R'   -'A'+ALPHA_START);
	
	set_bkg_tiles(0U, 8U, 20U, 1U, buffer);
	
	for (i = 0; i < 20; i++)
		buffer[i] = BLANK_TILE;
	
	buffer[6] = ('P'   -'A'+ALPHA_START);
	buffer[7] = ('L'   -'A'+ALPHA_START);
	buffer[8] = ('A'   -'A'+ALPHA_START);
	buffer[9] = ('Y'   -'A'+ALPHA_START);
	buffer[10] = ('I'   -'A'+ALPHA_START);
	buffer[11] = ('N'   -'A'+ALPHA_START);
	buffer[12] = ('G'   -'A'+ALPHA_START);
	buffer[13] = ALPHA_START+26; // !
	set_bkg_tiles(0U, 10U, 20U, 1U, buffer);


	
	SHOW_BKG;
	
	DISPLAY_ON;
	enable_interrupts();

	waitpadup();
	oldBtns = btns;
	btns = joypad();
	while (!(!(oldBtns & J_START) && btns & J_START)) {
		oldBtns = btns;
		btns = joypad();
		wait_vbl_done();
	}
}