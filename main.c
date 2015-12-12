#include "main.h"
#include "helper_1.h"

#include <gb/gb.h>
#include <rand.h>

// TODO LIST
// 1. Figure out the write exception on every startup in bgb. (Saving grace: Real hardware doesn't seem to care.)

#define BANK_GRAPHICS 1U
#define BANK_WORLD_DATA 2U
#define BANK_HELPER_1 3U

// This won't get confusing. Honest. I swear. &@#*!
UBYTE i, j;

UBYTE isMiniMode;
UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel;
UBYTE buffer[MAP_TILES_ACROSS*2];
UINT16 playerWorldTileStart;
UBYTE* currentMap;
void init_vars() {
	isMiniMode = 0U;
	playerWorldPos = 0U;
	playerWorldTileStart = get_map_tile_base_position();
	currentMap = world_0;
	btns = oldBtns = 0U;
	playerXVel = playerYVel = 0U;
	
	playerX = playerY = 32U;
}

void load_map() {
	SWITCH_ROM_MBC1(BANK_WORLD_DATA);
	
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
}

void init_screen() {
	
	disable_interrupts();
	DISPLAY_OFF;
	
	SWITCH_ROM_MBC1(BANK_GRAPHICS);
	set_bkg_data(0U, 100U, base_tiles);
	set_win_data(0U, 100U, base_tiles);
	set_sprite_data(0U, 64U, base_sprites);
	SPRITES_8x8;
	
	// Main char is first 4 sprites. (Though sometimse 1 will be used...)
	for (i = 0U; i < 4U; i++) 
		set_sprite_tile(i, i);
	
	load_map();

	SHOW_BKG;
	SHOW_SPRITES;
	
	DISPLAY_ON;
	enable_interrupts();
}

// Get the position of the top left corner of a room on the map.
// Shamelessly stolen from Isle Quest GB.
INT16 get_map_tile_base_position() {
	return ((playerWorldPos / 10U) * (MAP_TILE_ROW_WIDTH*MAP_TILE_ROW_HEIGHT)) + ((playerWorldPos % 10U) * MAP_TILES_ACROSS);
}

void main_game_loop() {
	
	SWITCH_ROM_MBC1(BANK_HELPER_1);
	
	oldBtns = btns;
	btns = joypad();
	handle_input();
	
	playerX += playerXVel;
	playerY += playerYVel;
	
	if (isMiniMode) {
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
	
	// Limit us to not-batnose-crazy speeds
	wait_vbl_done();
}

void main(void) {
	init_vars();
	init_screen();
	
	while(1) {
		main_game_loop();
	}
}