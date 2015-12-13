#include "helper_1.h"
#include "main.h"
#include <gb/gb.h>
#include <rand.h>

// Yes, this is a pretty lame way to organize this thing... it's a 48 hour coding marathon, put up with it, or go elsewhere! 

void handle_input() {
	
	if (!playerVelocityLock) {
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
		
		set_win_tiles(1U, 0U, 20U, 1U, buffer);
	}
	
}

void update_health() {
	for (i = 0; i < playerHealth; i++)
		buffer[i] = HEART_TILE;

	for (; i < 20U; i++)
		buffer[i] = BLANK_TILE;
	
	set_win_tiles(1U, 0U, 20U, 1U, buffer);

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

void do_player_movey_stuff() {
	// temp3 will be anything we might have collided with.
	if (isMiniMode && temp3 > FIRST_WATER_TILE && temp3 < FIRST_LOG_TILE) {
		playerXVel = -playerXVel;
		playerYVel = -playerYVel;
		temp1 = playerX + playerXVel;
		temp2 = playerY + playerYVel;
		playerVelocityLock = PLAYER_DAMAGE_TIME;
		playerHealth--;
		if (playerHealth == 0) {
			gameState = GAME_STATE_GAME_OVER;
			return;
		}
		update_health();
	}
	
	playerX = temp1;
	playerY = temp2;
		
	if (isMiniMode) {
		if (playerVelocityLock && (sys_time & PLAYER_BLINKY_INTERVAL) >> PLAYER_BLINKY_SHIFT)
			move_sprite(0U, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
		else
			move_sprite(0U, playerX, playerY);
		for (i = 1; i != 4U; i++)
			move_sprite(i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
		set_sprite_tile(0, ((sys_time & PLAYER_ANIM_INTERVAL) >> PLAYER_ANIM_SHIFT)); // HACK: We know this is 0, so don't add a base # to it.
	} else {
		for (i = 0U; i != 4U; i++) {
			move_sprite(i, playerX + (i/2U)*8U, playerY + (i%2U)*8U);
			set_sprite_tile(i, SPRITE_BIG + (((sys_time & PLAYER_ANIM_INTERVAL) >> PLAYER_ANIM_SHIFT)*4)+i);
		}
	}
	
	if (playerVelocityLock > 0)
		playerVelocityLock--;

}

void directionalize_sprites() {
	// Kind of bizarre, but it gives us a good variation.
	if (cycleCounter % 60U < MAX_SPRITES) {
		temp3 = rand() % 16U;
		if (temp3 > SPRITE_DIRECTION_DOWN) {
			if (temp3 < 9) {
				temp3 = SPRITE_DIRECTION_STOP;
			} else if (temp3 < 12) {
				temp4 = playerX + (spriteSize/2U);
				temp5 = sprites[temp1].x + (sprites[temp1].size/2U);
				if (temp4 < temp5)
					temp3 = isMiniMode ? SPRITE_DIRECTION_LEFT : SPRITE_DIRECTION_RIGHT;
				else
					temp3 = isMiniMode ? SPRITE_DIRECTION_RIGHT : SPRITE_DIRECTION_LEFT;
			} else {
				temp4 = playerY + (spriteSize/2U);
				temp5 = sprites[temp1].y + (sprites[temp1].size/2U);
				if (temp4 < temp5)
					temp3 = isMiniMode ? SPRITE_DIRECTION_UP : SPRITE_DIRECTION_DOWN;
				else
					temp3 = isMiniMode ? SPRITE_DIRECTION_DOWN : SPRITE_DIRECTION_UP;

			}
		}
		sprites[temp1].direction = temp3;
	}

}