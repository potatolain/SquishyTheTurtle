#include <gb/gb.h>
#include "intro.h"
#include "main.h"

void show_intro() {
	disable_interrupts();
	DISPLAY_OFF;
	HIDE_SPRITES;
	HIDE_WIN;
			
	set_bkg_tiles(0U, 0U, 20U, 20U, introwin_screens);

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