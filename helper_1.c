#include "helper_1.h"
#include "main.h"
#include <gb/gb.h>
#include <rand.h>

// Yes, this is a pretty lame way to organize this thing... it's a 48 hour coding marathon, put up with it, or go elsewhere! 

void handle_input() {
	
	oldBtns = btns;
	btns = joypad();
	
	if (!playerVelocityLock) {
		playerXVel = playerYVel = 0;

		if (btns & J_UP) {
			playerYVel = isMiniMode ? -PLAYER_MOVE_DISTANCE_FAST : -PLAYER_MOVE_DISTANCE_SLOW;
			playerDirection = SPRITE_DIRECTION_UP;
		} else if (btns & J_DOWN) {
			playerYVel = isMiniMode ? PLAYER_MOVE_DISTANCE_FAST : PLAYER_MOVE_DISTANCE_SLOW;
			playerDirection = SPRITE_DIRECTION_DOWN;
		}
		
		if (btns & J_LEFT) {
			playerXVel = isMiniMode ? -PLAYER_MOVE_DISTANCE_FAST : -PLAYER_MOVE_DISTANCE_SLOW;
			playerDirection = SPRITE_DIRECTION_LEFT;
		} else if (btns & J_RIGHT) {
			playerXVel = isMiniMode ? PLAYER_MOVE_DISTANCE_FAST : PLAYER_MOVE_DISTANCE_SLOW;
			playerDirection = SPRITE_DIRECTION_RIGHT;
		}
		
		if (!(oldBtns & J_A) && btns & J_A) {
			isMiniMode = !isMiniMode;
			if (isMiniMode) {
				playerX += 4U;
				playerY += 4U;
				spriteSize = 8U;
				// HACK: Manually set the sprite for our newly tinied turtle. We aren't moving, so this won't happen automatically.
				set_sprite_tile(0, 0);
			} else {
				playerX -= 4U;
				playerY -= 4U;
				spriteSize = 16U;
				// HACK: See above, we need it here too.
				for (i = 0; i < 4; i++)
					set_sprite_tile(i, SPRITE_BIG+i);
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
void update_egg() {
	// Overkill? nah...
	buffer[0] = EGG_TILE;
	buffer[1] = E_TILE;
	buffer[2] = G_TILE;
	buffer[3] = G_TILE;
	buffer[4] = BLANK_TILE;
	buffer[5] = (UBYTE)(NUMERIC_TILE+currentEggs);
	buffer[6] = SLASH_TILE;
	buffer[7] = (UBYTE)(NUMERIC_TILE+totalEggs);
	set_win_tiles(12U, 1U, 8U, 1U, buffer);
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

void turn_x_y_to_tile(UBYTE x, UBYTE y) {
	x -= 8;
	temp16 = playerWorldTileStart + (MAP_TILE_ROW_WIDTH * (((UINT16)y>>4U) - 1U)) + (((UINT16)x)>>4U);
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
	
	// Gross, stinky check to see if we're entirely within the teleporter tile.
	if (isMiniMode && currentEggs >= totalEggs) {
		turn_x_y_to_tile(temp1, temp2);
		temp16b = temp16;
		if (get_tile_at_pos(temp16) == TELEPORTER_TILE) {
			turn_x_y_to_tile(temp1+spriteSize, temp2);
			if (temp16 == temp16b) {
				turn_x_y_to_tile(temp1, temp2+spriteSize);
				if (temp16 == temp16b) {
					turn_x_y_to_tile(temp1+spriteSize, temp2+spriteSize);
					if (temp16 == temp16b) {
						// You're in. Barf bags are available to your left.
						currentLevelNum++;
						if (currentLevelNum > LAST_LEVEL) {
							gameState = GAME_STATE_WINNER; // Haha, you win, so you lose. FORGET YOU!! :D
						} else {
							gameState = GAME_STATE_LOAD;
						}
						return; // Run away!
					}
				}
			}
		}
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
		if (playerXVel + playerYVel != 0)
			set_sprite_tile(0, ((sys_time & PLAYER_ANIM_INTERVAL) >> PLAYER_ANIM_SHIFT) + ((playerDirection-1)<<1)); // HACK: We know this is 0, so don't add a base # to it.
	} else {
		for (i = 0U; i != 4U; i++) {
			move_sprite(i, playerX + (i/2U)*8U, playerY + (i%2U)*8U);
			if (playerXVel + playerYVel != 0U)
				set_sprite_tile(i, SPRITE_BIG + (((sys_time & PLAYER_ANIM_INTERVAL) >> PLAYER_ANIM_SHIFT)<<2U) + ((playerDirection-1U)<<3U) + i);
		}
	}
	
	if (playerVelocityLock > 0)
		playerVelocityLock--;

}

void directionalize_sprites() {
	// Kind of bizarre, but it gives us a good variation.
	if (sprites[temp1].type != SPRITE_TYPE_EGG) { // Eggs can bug right off!
		if (cycleCounter % 60U < MAX_SPRITES) {
			temp3 = rand() % 32U;
			if (temp3 > SPRITE_DIRECTION_DOWN) {
				if (temp3 < 9) {
					temp3 = SPRITE_DIRECTION_STOP;
				} else if (temp3 < 21) {
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
		
		temp4 = sprites[temp1].x;
		temp5 = sprites[temp1].y;

		switch (sprites[temp1].direction) {
			case SPRITE_DIRECTION_LEFT: 
				temp4 -= SPIDER_SPEED;
				break;
			case SPRITE_DIRECTION_RIGHT:
				temp4 += SPIDER_SPEED;
				break;
			case SPRITE_DIRECTION_UP:
				temp5 -= SPIDER_SPEED;
				break;
			case SPRITE_DIRECTION_DOWN:
				temp5 += SPIDER_SPEED;
				break;
		}
	} else {
		temp4 = sprites[temp1].x;
		temp5 = sprites[temp1].y;
	}
}

void test_sprite_collision() {
	for (i = 0U; i < MAX_SPRITES; i++) {
		if (playerX < sprites[i].x + sprites[i].size && playerX + spriteSize > sprites[i].x && 
				playerY < sprites[i].y + sprites[i].size && playerY + spriteSize > sprites[i].y) {
			
			if (sprites[i].type == SPRITE_TYPE_EGG) {
				currentEggs++;
				// update egg positions
				temp5 = 1;
				temp5 >>= (playerWorldPos % 8);
				eggStatus[playerWorldPos/8] = eggStatus[playerWorldPos/8] | temp5;
				
				sprites[i].x = SPRITE_OFFSCREEN;
				sprites[i].y = SPRITE_OFFSCREEN;
				update_egg();
				return;
			} else {
				playerHealth--;
				if (playerHealth == 0) {
					gameState = GAME_STATE_GAME_OVER;
					return;
				}

				update_health();
				playerVelocityLock = PLAYER_DAMAGE_TIME;
				if (playerXVel == 0 && playerYVel == 0) {
					playerYVel = PLAYER_MOVE_DISTANCE_FAST;
				} else {
					playerYVel = 0U-playerYVel;
					playerXVel = 0U-playerXVel;
				}
				return;
			}
		}
	}
}

void clear_extra_sprites() {
	while (temp2 != MAX_SPRITES) {
		// Fill in the rest -- both in actual sprites and in our structs.
		for (i = 0U; i < 4U; i++)
			move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
		
		sprites[temp2].type = SPRITE_TYPE_NONE;
		sprites[temp2].x = sprites[temp2].y = SPRITE_OFFSCREEN;
		sprites[temp2].size = 0U;
		temp2++;
	}

}

void init_vars() {
	isMiniMode = 1U;
	playerWorldPos = 0U;
	playerWorldTileStart = get_map_tile_base_position();
	btns = oldBtns = 0U;
	playerXVel = playerYVel = 0U;
	spriteSize = 8U;
	
	playerHealth = 5U;
	gameState = GAME_STATE_RUNNING;
	playerVelocityLock = 0U;
	currentEggs = 0U;
	currentLevelNum = 0U;
	for (i = 0; i < 13; i++)
		eggStatus[i] = 0;

}

void finish_init_screen() {
	update_egg();
	
	scroll_bkg(0U, 0U);
	SPRITES_8x8;
	
	// Main char is first 4 sprites. (Though sometimes 1 will be used...)
	for (i = 0U; i < 4U; i++) 
		set_sprite_tile(i, i);
	
	SHOW_BKG;
	SHOW_SPRITES;

	move_win(0, 128);
	SHOW_WIN;
	
	DISPLAY_ON;
	enable_interrupts();

}

void test_for_egg() {
	if (sprites[temp2].type == SPRITE_TYPE_EGG) {
		temp5 = 1U;
		temp5 >>= (playerWorldPos % 8);
		if (eggStatus[playerWorldPos/8] & temp5) {
			sprites[temp2].x = sprites[temp2].y = SPRITE_OFFSCREEN;
			sprites[temp2].type = SPRITE_TYPE_NONE;
		}
	}

}

void move_sprites_for_load() {
	// Leaving room for 4 sprite... sprites, but for now, we'll just use the first.
	set_sprite_tile(WORLD_SPRITE_START + (temp2 << 2U), ENEMY_SPRITE_START + (sprites[temp2].type << 2U));
	move_sprite(WORLD_SPRITE_START + (temp2 << 2U), sprites[temp2].x, sprites[temp2].y);

}

UBYTE get_collision_with_temp3() {
	if (temp3 == WALKABLE_LOG_TILE) {
		return !isMiniMode; // Tiny you can walk. Big you can't. GET STUCKED!!
	}
	if (temp3 > FIRST_WATER_TILE && temp3 < FIRST_LOG_TILE) {
		return isMiniMode;
	}
	if (temp3 > FIRST_SOLID_TILE - 1U) {
		return 1;
	}
	return 0;

}