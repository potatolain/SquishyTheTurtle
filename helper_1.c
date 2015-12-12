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
	
	// HAAAAAAAAAX
	if (!(oldBtns & J_SELECT) && btns & J_SELECT) {
		playerHealth = (playerHealth+1)%(PLAYER_MAX_HEALTH+1);
		update_health();
	}
}

void update_health() {
	for (i = 0; i < playerHealth; i++)
		buffer[i] = HEART_TILE;

	for (; i < (PLAYER_MAX_HEALTH+1); i++)
		buffer[i] = BLANK_TILE;
	
	set_win_tiles(1U, 0U, PLAYER_MAX_HEALTH+1, 0U, buffer);
}