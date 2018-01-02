#include <gb/gb.h>
#include "intro.h"
#include "main.h"
#include "sram.h"
#include "title.h"


extern UBYTE title_tiles[];
extern UINT32 gameTimer;

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
	set_bkg_tiles(14U, 10U, 5U, 1U, buffer);
	
	for (i = 0U; i != 5U; ++i)
		buffer[i] = sram_numberOfEnds[4-i] + NUM_START;
	for (i = 0U; i != 5U; ++i)
		if (buffer[i] == NUM_START)
			buffer[i] = BLANK_TILE;
		else
			break;
	set_bkg_tiles(14U, 11U, 5U, 1U, buffer);

	// Okay, this part is hard. We have the number of interrupts that have occurred since the player hit start. 
	// There are 59.7 (or 60.2 or so on SGB) in a second... so we're just gonna call it 60. 
	//adjustedTimer = gameTimer / 60; // Okay, that's seconds.
	//temp1 = adjustedTimer % 60; // Seconds
	//temp2 = (adjustedTimer / 60) % 60; // Minutes
	//temp3 = (adjustedTimer / 60 / 60) % 60; // Hours!?
	// If you took more than an hour... sorry, you took 99:99.
	if (timerHour > 0) {
		timerMinute = 99;
		timerSecond = 99;
	}
	// Ok now put buffered time into the thingie
	//buffer[0] = '0';
	//buffer[1] = '0';
	//buffer[2] = 'h';
	buffer[0] = NUM_START + (timerMinute / 10);
	buffer[1] = NUM_START + (timerMinute % 10);
	buffer[2] = 'm' - 'a' + LC_ALPHA_START;
	buffer[3] = NUM_START + (timerSecond / 10);
	buffer[4] = NUM_START + (timerSecond % 10);
	buffer[5] = 's' - 'a' + LC_ALPHA_START;
	set_bkg_tiles(13U, 12U, 6U, 1U, buffer);


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