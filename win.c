#include <gb/gb.h>
#include "intro.h"
#include "main.h"
#include "sram.h"
#include "title.h"


extern UBYTE title_tiles[];

void show_win() {
	disable_interrupts();
	DISPLAY_OFF;
	HIDE_SPRITES;
	HIDE_WIN;
			
	set_bkg_data(0U, 128U, title_tiles); // NOTE: This shouldn't work, but our banking is screwed up so it does.
	set_bkg_tiles(0U, 0U, 20U, 20U, introwin_screens+(20U*18U));

	for (i = 0; i != 20; ++i) {
		buffer[i] = BLANK_TILE;
	}

	ENABLE_RAM_MBC1;

	for (i = 0U; i != 5U; ++i)
		buffer[i] = sram_numberOfStarts[4-i] + NUM_START;
	for (i = 0U; i != 5U; ++i)
		if (buffer[i] == NUM_START)
			buffer[i] = BLANK_TILE;
		else
			break;
	set_bkg_tiles(14U, 11U, 5U, 1U, buffer);
	
	for (i = 0U; i != 5U; ++i)
		buffer[i] = sram_numberOfEnds[4-i] + NUM_START;
	for (i = 0U; i != 5U; ++i)
		if (buffer[i] == NUM_START)
			buffer[i] = BLANK_TILE;
		else
			break;
	set_bkg_tiles(14U, 12U, 5U, 1U, buffer);



	DISABLE_RAM_MBC1;


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