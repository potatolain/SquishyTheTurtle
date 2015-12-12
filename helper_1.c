#include "helper_1.h"
#include "main.h"
#include <gb/gb.h>

// Yes, this is a pretty lame way to organize this thing... it's a 48 hour coding marathon, put up with it, or go elsewhere! 

void handle_input() {
	playerXVel = playerYVel = 0;
	
	if (btns & J_UP)
		playerYVel = -PLAYER_MOVE_DISTANCE;
	else if (btns & J_DOWN)
		playerYVel = PLAYER_MOVE_DISTANCE;
	
	if (btns & J_LEFT)
		playerXVel = -PLAYER_MOVE_DISTANCE;
	else if (btns & J_RIGHT)
		playerXVel = PLAYER_MOVE_DISTANCE;
	
	if (!(oldBtns & J_A) && btns & J_A) {
		isMiniMode = !isMiniMode;
		if (isMiniMode) {
			playerX += 4U;
			playerY += 4U;
			spriteSize = 8U;
		} else {
			playerX -= 4U;
			playerY -= 4U;
			spriteSize = 16U;
		}
	}
	
	if (!(oldBtns & J_START) && btns & J_START) {
		gameState = GAME_STATE_PAUSED;
		// Darken the screen
		BGP_REG = 0xf9;
		for (i = 0; i < 6; i++)
			buffer[i] = BLANK_TILE;
		
		buffer[i++] = DASH_TILE;
		buffer[i++] = BLANK_TILE;
		
		buffer[i++] = PAUSE_TILE;
		buffer[i++] = PAUSE_TILE+1U;
		buffer[i++] = PAUSE_TILE+2U;
		buffer[i++] = PAUSE_TILE+3U;
		buffer[i++] = PAUSE_TILE+4U;
		
		buffer[i++] = BLANK_TILE;
		buffer[i++] = DASH_TILE;
		
		for (; i < 20U; i++)
			buffer[i] = BLANK_TILE;
		
		set_win_tiles(1U, 0U, 20U, 0U, buffer);
		for (i = 0; i < 20U; i++)
			buffer[i] = BLANK_TILE;
		set_win_tiles(1U, 1U, 20U, 0U, buffer);
	}
	
	// HAAAAAAAAAX
	if (!(oldBtns & J_SELECT) && btns & J_SELECT) {
		playerHealth = (playerHealth+1)%(PLAYER_MAX_HEALTH+1);
		update_health();
	}
}

void update_health() {
	for (i = 0; i < playerHealth; i++)
		buffer[i] = HEART_TILE;

	for (; i < 20U; i++)
		buffer[i] = BLANK_TILE;
	
	set_win_tiles(1U, 0U, 19U, 0U, buffer);
	for (i = 0; i < 20U; i++)
		buffer[i] = BLANK_TILE;
	set_win_tiles(1U, 1U, 19U, 0U, buffer);

}

void pause_loop() {
	
	oldBtns = btns;
	btns = joypad();
	
	if (!(oldBtns & J_START) && btns & J_START) {
		gameState = GAME_STATE_RUNNING;
		BGP_REG = 0xe4;
		update_health();
	}
	wait_vbl_done();
}