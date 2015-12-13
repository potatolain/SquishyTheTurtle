#include "main.h"
#include "helper_1.h"
#include "title.h"
#include "sprite.h"
#include "graphics/world_0_sprites.h"

#include <gb/gb.h>
#include <rand.h>

// TODO: 
// - Create levels
// - Animate exit
// - SFX?

#define BANK_GRAPHICS 1U
#define BANK_WORLD_DATA 2U
#define BANK_HELPER_1 3U
#define BANK_TITLE 4U
#define BANK_SPRITE_DATA 5U

// This won't get confusing. Honest. I swear. &@#*!
UBYTE i, j;

UBYTE isMiniMode;
UBYTE temp1, temp2, temp3, temp4, temp5;
UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, spriteSize, gameState, playerVelocityLock, cycleCounter, currentEggs, totalEggs;
UBYTE playerHealth;
UBYTE buffer[20U];
UBYTE eggStatus[13U]; // 1 bit per tile... 
UINT16 playerWorldTileStart, temp16, temp16b;
UBYTE* currentMap;
UBYTE* tempPointer; 
enum SPRITE_DIRECTION playerDirection;
// Pointers to pointers to pointers to pointers to pointers to pointers to pointers to pointers to pointers to pointers to...
UBYTE* * * currentMapSprites;

struct SPRITE sprites[6];

void load_map() {
	SWITCH_ROM_MBC1(BANK_WORLD_DATA);
	currentMap = world_0;
	currentMapSprites = world_0_sprites;

	playerWorldTileStart = get_map_tile_base_position();
	
	// This is efficient. I swear! NOT AT ALL AWFUL. IN ANY WAY. NOPE.
	for (i = 0U; i != MAP_TILES_DOWN; i++) {
		for (j = 0U; j != MAP_TILES_ACROSS; j++) {
			buffer[j*2U] = currentMap[playerWorldTileStart + j] * 4U; // TODO: Bit shifts > multiplication. Our compiler's not smart enough to convert for us..
			buffer[j*2U+1U] = buffer[j*2U]+2U;
		}
		set_bkg_tiles(0U, i*2U, 20U, 1U, buffer);
		
		for (j = 0U; j != MAP_TILES_ACROSS*2; j++) {
			buffer[j]++;
		}
		set_bkg_tiles(0U, i*2U+1U, 20U, 1U, buffer);
		playerWorldTileStart += MAP_TILE_ROW_WIDTH;
	}
	playerWorldTileStart = get_map_tile_base_position(); // Clean up after yourself darnit!!
	
	
	SWITCH_ROM_MBC1(BANK_SPRITE_DATA);
	totalEggs = world_0_data[0];

	tempPointer = currentMapSprites[playerWorldPos];
	temp1 = 0x00; // Generic data
	temp2 = 0U; // Position
	while(temp2 != MAX_SPRITES) {
		temp1 = tempPointer++[0];
		if (temp1 == 255U)
			break;
		
		// Temp1 is our position.. convert to x/y
		sprites[temp2].x = ((temp1 % 10U) << 4U) + 12U; // Add 4 to place at center of tile, add 8 to deal with offset by 1.
		sprites[temp2].y = ((temp1 / 10U) << 4U) + 20U; // Add 16 so the first tile = 16, then add 4 to center within tile.
		sprites[temp2].size = 8U;

		sprites[temp2].type = tempPointer++[0];
		// Annihilate eggs we've already fetched.
		SWITCH_ROM_MBC1(BANK_HELPER_1);
		test_for_egg();
		SWITCH_ROM_MBC1(BANK_SPRITE_DATA);
		
		// Apply it to some real-world sprites too!
		// Leaving room for 4 sprite... sprites, but for now, we'll just use the first.
		set_sprite_tile(WORLD_SPRITE_START + (temp2 << 2U), ENEMY_SPRITE_START + (sprites[temp2].type << 2U));
		move_sprite(WORLD_SPRITE_START + (temp2 << 2U), sprites[temp2].x, sprites[temp2].y);
		temp2++;
	}
	
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	clear_extra_sprites();
}

UBYTE get_tile_at_pos(UINT16 position) {
	UBYTE temp;
	SWITCH_ROM_MBC1(BANK_WORLD_DATA);
	temp = currentMap[position];
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	return temp;
}

void init_screen() {
	
	disable_interrupts();
	DISPLAY_OFF;
	
	SWITCH_ROM_MBC1(BANK_GRAPHICS);
	set_bkg_data(0U, 128U, base_tiles);
	set_win_data(0U, 128U, base_tiles);
	set_sprite_data(0U, 64U, base_sprites);

	load_map();
	
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	finish_init_screen();
}

// Get the position of the top left corner of a room on the map.
// Shamelessly stolen from Isle Quest GB.
INT16 get_map_tile_base_position() {
	return ((playerWorldPos / 10U) * (MAP_TILE_ROW_WIDTH*MAP_TILE_ROW_HEIGHT)) + ((playerWorldPos % 10U) * MAP_TILES_ACROSS);
}

// Returns collision, and ALSO SETS temp3 to whatever was collided with.
UINT16 test_collision(UBYTE x, UBYTE y) {
	// This offsets us by one tile to get us in line with 0-7= tile 0, 8-f = tile 1, etc...
	x -= 8;
	temp16 = playerWorldTileStart + (MAP_TILE_ROW_WIDTH * (((UINT16)y>>4U) - 1U)) + (((UINT16)x)>>4U);
	temp3 = currentMap[temp16];
	
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

void move_sprites() {
	temp1 = cycleCounter % MAX_SPRITES;
	if (sprites[temp1].type == SPRITE_TYPE_NONE)
		return;
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	directionalize_sprites();
		
	SWITCH_ROM_MBC1(BANK_WORLD_DATA);	
	// Now, we test collision with our temp4 and temp5
	if (sprites[temp1].type != SPRITE_TYPE_EGG) {
		
		if (sprites[temp1].direction == SPRITE_DIRECTION_STOP)
			return;

		
		if (sprites[temp1].direction == SPRITE_DIRECTION_LEFT || sprites[temp1].direction == SPRITE_DIRECTION_RIGHT) {
			if (temp4+sprites[temp1].size >= SCREEN_WIDTH || temp4 <= 4U) {
				temp4 = sprites[temp1].x;
			} else {
				if (sprites[temp1].direction == SPRITE_DIRECTION_RIGHT) {
					if (test_collision(temp4+sprites[temp1].size, temp5) || test_collision(temp4 + sprites[temp1].size, temp5+sprites[temp1].size)) {
						temp4 = sprites[temp1].x;
					}
				} else {
					if (test_collision(temp4-1U, temp5) || test_collision(temp4-1U, temp5+sprites[temp1].size)) {
						temp4 = sprites[temp1].x;
					}
				}
			}
		}
		
		if (sprites[temp1].direction == SPRITE_DIRECTION_UP || sprites[temp1].direction == SPRITE_DIRECTION_DOWN) {
			if (temp5+sprites[temp1].size >= SCREEN_HEIGHT || temp5 <= 4U) {
				temp5 = sprites[temp1].y;
			} else {
				if (sprites[temp1].direction == SPRITE_DIRECTION_DOWN) {
					if (test_collision(temp4, temp5+sprites[temp1].size) || test_collision(temp4+sprites[temp1].size, temp5 + sprites[temp1].size)) {
						temp5 = sprites[temp1].y;
					}
				} else {
					if (test_collision(temp4, temp5) || test_collision(temp4 + sprites[temp1].size, temp5)) {
						temp5 = sprites[temp1].y;
					}
				}
			}
		}
		
		// Okay, you can move.
		sprites[temp1].x = temp4;
		sprites[temp1].y = temp5;
		set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U), ENEMY_SPRITE_START + (sprites[temp1].type << 2U) + ((sys_time & SPRITE_ANIM_INTERVAL) >> SPRITE_ANIM_SHIFT));
	} else {
		// You're an egg.
		set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U), EGG_SPRITE);
	}
	
	move_sprite(WORLD_SPRITE_START + (temp1 << 2U), temp4, temp5);
	// TODO: Other 4 for larger sprites.
}

void main_game_loop() {
	
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	
	oldBtns = btns;
	btns = joypad();
	handle_input();
	
	if (!playerVelocityLock) {
		test_sprite_collision();
	}
	
	move_sprites();
	
	SWITCH_ROM_MBC1(BANK_WORLD_DATA);
	temp1 = playerX + playerXVel;
	temp2 = playerY + playerYVel;
	temp3 = 0U;
	if (playerXVel != 0) {
		if (temp1+spriteSize >= SCREEN_WIDTH) {
			playerX = 8U + PLAYER_MOVE_DISTANCE;
			playerWorldPos++;
			load_map();
			return;
		} else if (temp1 <= 4U) {
			playerX = SCREEN_WIDTH - spriteSize - PLAYER_MOVE_DISTANCE;
			playerWorldPos--;
			load_map();
			return;
		} else {
			if (playerXVel == PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1+spriteSize, temp2) || test_collision(temp1+spriteSize, temp2+spriteSize)) {
					temp1 = playerX;
				}
			} else {
				if (test_collision(temp1-1U, temp2) || test_collision(temp1-1U, temp2+spriteSize)) {
					temp1 = playerX;
				}
			}
		}
	}
	
	if (playerYVel != 0) {
		if (temp2+spriteSize >= SCREEN_HEIGHT) {
			playerY = spriteSize + PLAYER_MOVE_DISTANCE;
			playerWorldPos += 10U;
			load_map();
			return;
		} else if (temp2 <= 4U) {
			playerY = (SCREEN_HEIGHT - STATUS_BAR_HEIGHT) - PLAYER_MOVE_DISTANCE;
			playerWorldPos -= 10U;
			load_map();
			return;
		} else {
			if (playerYVel == PLAYER_MOVE_DISTANCE) {
				if (test_collision(temp1, temp2+spriteSize) || test_collision(temp1+spriteSize, temp2+spriteSize)) {
					temp2 = playerY;
				}
			} else {
				if (test_collision(temp1, temp2) || test_collision(temp1+spriteSize, temp2)) {
					temp2 = playerY;
				}
			}
		}
	}
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	do_player_movey_stuff();
	
	// Limit us to not-batnose-crazy speeds
	wait_vbl_done();
}

void main(void) {
	startOver:
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	init_vars();
	
	SWITCH_ROM_MBC1(BANK_TITLE);
	show_title();
	initrand(sys_time);
	
	init_screen();
	
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	update_health();
	
	while(1) {
		switch (gameState) {
			case GAME_STATE_RUNNING:
				main_game_loop();
				break;
			case GAME_STATE_PAUSED:
				SWITCH_ROM_MBC1(BANK_HELPER_1);
				pause_loop();
				break;
			case GAME_STATE_GAME_OVER:
				SWITCH_ROM_MBC1(BANK_TITLE);
				show_game_over();
				goto startOver; // FREEDOM!!!!!!!! Start over.
			case GAME_STATE_WINNER:
				SWITCH_ROM_MBC1(BANK_TITLE);
				show_win_screen();
				goto startOver;
		}
		cycleCounter++;
	}
}