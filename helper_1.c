#include "helper_1.h"
#include "main.h"
#include <gb/gb.h>
#include <rand.h>
#include "sram.h"

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
				make_shrink_sound();
			} else {
				playerX -= 4U;
				playerY -= 4U;
				spriteSize = 16U;
				// HACK: See above, we need it here too.
				for (i = 0; i < 4; i++)
					set_sprite_tile(i, SPRITE_BIG+i);
				make_grow_sound();
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
		
		make_pause_sound();
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
		make_unpause_sound();
	}
	wait_vbl_done();
}

void turn_x_y_to_tile(UBYTE x, UBYTE y) {
	x -= 8;
	temp16 = playerWorldTileStart + (MAP_TILE_ROW_WIDTH * (((UINT16)y>>4U) - 1U)) + (((UINT16)x)>>4U);
}

void do_player_movey_stuff() {
	// temp3 will be anything we might have collided with.
	if (isMiniMode && !playerVelocityLock && temp3 > FIRST_WATER_TILE && temp3 < FIRST_LOG_TILE) {
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
		make_player_hurt_noise();
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
						make_clear_level_noise();
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
			if (playerVelocityLock && (sys_time & PLAYER_BLINKY_INTERVAL) >> PLAYER_BLINKY_SHIFT) {
				move_sprite(i, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
			} else {
				move_sprite(i, playerX + (i/2U)*8U, playerY + (i%2U)*8U);
				if (playerXVel + playerYVel != 0U)
					set_sprite_tile(i, SPRITE_BIG + (((sys_time & PLAYER_ANIM_INTERVAL) >> PLAYER_ANIM_SHIFT)<<2U) + ((playerDirection-1U)<<3U) + i);
			}
		}
	}
	
	if (playerVelocityLock > 0)
		playerVelocityLock--;
	
	// Weird place to do this, but this will be called once per frame. While we're in this bank...
	animate_exit();
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
					if ((sprites[temp1].type == SPRITE_TYPE_SPIDER && (temp4 < temp5)) || (sprites[temp1].type == SPRITE_TYPE_CRAB && (temp4 > temp5)))
						temp3 = isMiniMode ? SPRITE_DIRECTION_LEFT : SPRITE_DIRECTION_RIGHT;
					else
						temp3 = isMiniMode ? SPRITE_DIRECTION_RIGHT : SPRITE_DIRECTION_LEFT;
				} else {
					temp4 = playerY + (spriteSize/2U);
					temp5 = sprites[temp1].y + (sprites[temp1].size/2U);
					if ((sprites[temp1].type == SPRITE_TYPE_SPIDER && (temp4 < temp5)) || (sprites[temp1].type == SPRITE_TYPE_CRAB && (temp4 > temp5)))
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
	
	// We're about to do sprite collision, but have 0 space in that bank. So... let's tell ourselves we're going to do sprite collisions.
	collisionsAreForPlayer = 0;
	
	// Speaking of things that don't fit in the bank they belong... let's figure out the width of our sprite.
	temp6 = sprites[temp1].size;
	if (sprites[temp1].type == SPRITE_TYPE_CRAB)
		temp6 = 16U;
}

void test_sprite_collision() {
	UBYTE spriteWidth, spriteHeight;
	for (i = 0U; i < MAX_SPRITES; i++) {
		spriteWidth = sprites[i].size;
		spriteHeight = sprites[i].size;
		if (sprites[i].type == SPRITE_TYPE_CRAB) {
			spriteWidth = 16U; // Crabs are 16x8. Figure that out!
		}
		if (playerX < sprites[i].x + spriteWidth && playerX + spriteSize > sprites[i].x && 
				playerY < sprites[i].y + spriteHeight && playerY + spriteSize > sprites[i].y) {
			
			if (sprites[i].type == SPRITE_TYPE_EGG) {
				currentEggs++;
				// update egg positions
				temp5 = 1;
				temp5 <<= (playerWorldPos % 8);
				eggStatus[playerWorldPos/8] = eggStatus[playerWorldPos/8] | temp5;
				
				sprites[i].x = SPRITE_OFFSCREEN;
				sprites[i].y = SPRITE_OFFSCREEN;
				update_egg();
				make_egg_noise();
				return;
			} else if (!playerVelocityLock) {
				playerHealth--;
				make_player_hurt_noise();
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
	gameState = GAME_STATE_STARTUP;
	playerVelocityLock = 0U;
	currentEggs = 0U;
	currentLevelNum = START_LEVEL;
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
		temp5 <<= (playerWorldPos % 8);
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
	if (sprites[temp2].type == SPRITE_TYPE_CRAB) {
		set_sprite_tile(WORLD_SPRITE_START + (temp2 << 2u) + 1, ENEMY_SPRITE_START + (sprites[temp2].type << 2u)+1);
		move_sprite(WORLD_SPRITE_START + (temp2 <<2u) + 1, sprites[temp2].x + 8u, sprites[temp2].y);		
	} else {
		set_sprite_tile(WORLD_SPRITE_START + (temp2 << 2u) + 1, 0);
		move_sprite(WORLD_SPRITE_START + (temp2 <<2u) + 1, sprites[temp2].x, SPRITE_OFFSCREEN);
	}

}

UBYTE get_collision_with_temp3() {
	if (temp3 == WALKABLE_LOG_TILE) {
		return !isMiniMode; // Tiny you can walk. Big you can't. GET STUCKED!!
	}
	if (collisionsAreForPlayer && temp3 > FIRST_WATER_TILE && temp3 < FIRST_LOG_TILE) {
		return isMiniMode;
	}
	if (temp3 > FIRST_SOLID_TILE - 1U) {
		return 1;
	}
	return 0;

}

void animate_exit() {
	if (currentEggs < totalEggs || exitPositionX == 255U || exitPositionY == 255U)
		return;
	
	temp1 = ((sys_time & EXIT_ANIM_INTERVAL) >> EXIT_ANIM_SHIFT) << 2; // add 0/4
	
	buffer[0] = (TELEPORTER_TILE_ANIM_1<<2) + temp1;
	buffer[2] = buffer[0]+1;
	buffer[1] = buffer[2]+1;
	buffer[3] = buffer[1]+1;
	set_bkg_tiles(exitPositionX, exitPositionY, 2U, 2U, buffer);
}

void write_map_to_memory() {
	set_bkg_tiles(0U, i*2U, 20U, 1U, buffer);
	
	for (j = 0U; j != MAP_TILES_ACROSS*2; j++) {
		buffer[j]++;
	}
	set_bkg_tiles(0U, i*2U+1U, 20U, 1U, buffer);
	playerWorldTileStart += MAP_TILE_ROW_WIDTH;

}

void move_enemy_sprite() {
	sprites[temp1].x = temp4;
	sprites[temp1].y = temp5;
	if (sprites[temp1].type == SPRITE_TYPE_CRAB) {
		if (sprites[temp1].direction != SPRITE_DIRECTION_STOP) {
			temp7 = (((sys_time & SPRITE_ANIM_INTERVAL) >> SPRITE_ANIM_SHIFT)<<1U);
		} else {
			temp7 = 0;
		}
		// The patchwork crab... other move is in main()
		move_sprite(WORLD_SPRITE_START + (temp1 << 2U)+1U, temp4+8U, temp5);
		set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U), CRAB_SPRITE + temp7);
		set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U)+1U, CRAB_SPRITE + 1U + temp7);
	} else {
		if (sprites[temp1].direction != SPRITE_DIRECTION_STOP) {
			temp7 = ((sys_time & SPRITE_ANIM_INTERVAL) >> SPRITE_ANIM_SHIFT);
		} else {
			temp7 = 0;
		}
		set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U), ENEMY_SPRITE_START + (sprites[temp1].type << 2U) + temp7);
		// This is just getting scary bad...
		move_sprite(WORLD_SPRITE_START + (temp1 << 2U)+1U, SPRITE_OFFSCREEN, SPRITE_OFFSCREEN);
	}
	
}

void init_sram() {
	ENABLE_RAM_MBC1;
	SWITCH_RAM_MBC1(0);		
	if (sram_magicByte != MAGIC_BYTE) {
		sram_magicByte = MAGIC_BYTE;
		sram_numberOfStarts[0] = 0U;
		sram_numberOfStarts[1] = 0U;
		sram_numberOfStarts[2] = 0U;
		sram_numberOfStarts[3] = 0U;
		sram_numberOfStarts[4] = 0U;
		sram_numberOfEnds[0] = 0U;
		sram_numberOfEnds[1] = 0U;
		sram_numberOfEnds[2] = 0U;
		sram_numberOfEnds[3] = 0U;
		sram_numberOfEnds[4] = 0U;
	}
	DISABLE_RAM_MBC1;	
}

// TODO: This works on vba, fails miserably on BGB. Works on console as well. Assuming bgb is wrong. Would be nice to have it work, but it's lower priority than real consoles right now...
void inc_starts() {
	ENABLE_RAM_MBC1;
	for (i = 0; i != 5; i++) {
		++sram_numberOfStarts[i];
		if (sram_numberOfStarts[i] > 9U) {
			sram_numberOfStarts[i] -= 10U;
		} else {
			break;
		}
	}
	DISABLE_RAM_MBC1;
}

void inc_ends() {
	ENABLE_RAM_MBC1;
	for (i = 0; i != 5; i++) {
		++sram_numberOfEnds[i];
		if (sram_numberOfEnds[i] > 9U) {
			sram_numberOfEnds[i] -= 10U;
		} else {
			break;
		}
	}
	DISABLE_RAM_MBC1;
}


void make_player_hurt_noise() {
	
	NR52_REG = 0x80;
	NR51_REG = 0x88;
	NR50_REG = 0x77;

	NR41_REG = 0x10;
	NR42_REG = 0xe1; 
	NR43_REG = 0x25;
	NR44_REG = 0x87;
}

void make_clear_level_noise() {
	
	NR52_REG = 0x80;
	NR51_REG = 0x11;
	NR50_REG = 0x77;

	NR10_REG = 0x76;
	NR11_REG = 0x10;
	NR12_REG = 0xF3;
	NR13_REG = 0x00;
	NR14_REG = 0x87;
}

void make_egg_noise() {
	NR52_REG = 0x80;
	NR51_REG = 0x11;
	NR50_REG = 0x77;

	NR10_REG = 0x16;
	NR11_REG = 0x10;
	NR12_REG = 0xF3;
	NR13_REG = 0x00;
	NR14_REG = 0x87;
}

void make_shrink_sound() {
	NR52_REG = 0x80;
	NR51_REG = 0x11;
	NR50_REG = 0x77;

	NR10_REG = 0x36;
	NR11_REG = 0x10;
	NR12_REG = 0xF3;
	NR13_REG = 0x00;
	NR14_REG = 0x87;
}

void make_grow_sound() {
	NR52_REG = 0x80;
	NR51_REG = 0x11;
	NR50_REG = 0x77;

	NR10_REG = 0x36;
	NR11_REG = 0x8b;
	NR12_REG = 0xF3;
	NR13_REG = 0x00;
	NR14_REG = 0x87;
}

void make_pause_sound() {
	NR52_REG = 0x80;
	NR51_REG = 0x11;
	NR50_REG = 0x77;

	NR10_REG = 0x17;
	NR11_REG = 0xF6;
	NR12_REG = 0xF3;
	NR13_REG = 0x00;
	NR14_REG = 0x87;
}

void make_unpause_sound() {
	NR52_REG = 0x80;
	NR51_REG = 0x11;
	NR50_REG = 0x77;

	NR10_REG = 0x17;
	NR11_REG = 0xF6;
	NR12_REG = 0xF3;
	NR13_REG = 0x00;
	NR14_REG = 0x87;
}