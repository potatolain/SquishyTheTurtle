#ifndef MAIN_H
#define MAIN_H

#include <gb/gb.h>
#include "sprite.h"

#define GAME_STATE_RUNNING 0U
#define GAME_STATE_PAUSED 1U
#define GAME_STATE_TITLE 2U
#define GAME_STATE_GAME_OVER 3U

#define SPRITE_SMALL 0
#define SPRITE_BIG 8
#define FIRST_SOLID_TILE 7U
#define FIRST_WATER_TILE 8U
#define FIRST_LOG_TILE 15U
#define WALKABLE_LOG_TILE 15U

#define PLAYER_MAX_HEALTH 5U
#define PLAYER_DAMAGE_TIME 25U

#define BLANK_TILE 0U
#define HEART_TILE 96U
#define PAUSE_TILE 91U
#define DASH_TILE 90U
#define E_TILE 95U
#define G_TILE 97U
#define EGG_TILE 109U
#define SLASH_TILE 108U
#define NUMERIC_TILE 98U
#define TELEPORTER_TILE 5U

#define PLAYER_ANIM_INTERVAL 0x08U // %00000100
#define PLAYER_ANIM_SHIFT 3U
#define PLAYER_BLINKY_INTERVAL 0x04U
#define PLAYER_BLINKY_SHIFT 2U
#define PLAYER_MOVE_DISTANCE 2U

#define MAP_TILES_ACROSS 10U
#define MAP_TILES_DOWN 8U
#define MAP_TILE_ROW_WIDTH 100U
#define MAP_TILE_ROW_HEIGHT 8U 

#define SCREEN_WIDTH 160U
#define SCREEN_HEIGHT 140U
#define STATUS_BAR_HEIGHT 16U

#define SPRITE_OFFSCREEN 0xEF

extern UBYTE base_tiles[];
extern UBYTE base_sprites[];
extern UBYTE world_0[];

extern UBYTE buffer[20U];

extern UINT16 sys_time;

INT16 get_map_tile_base_position();
UBYTE get_tile_at_pos(UINT16 position);


// Yep... these are all globals. Limited memory, gameboy, poor compiler support for local vars, etc.
// If this is grossing you out, please stop thinking about it like C. Think about it like the assembly behind it,
// and also keep in mind that we're working in a very limited system. Tricks will be necessary.
extern UBYTE i, j;
extern UBYTE playerWorldPos, playerX, playerY, btns, oldBtns, playerXVel, playerYVel, isMiniMode, spriteSize, playerHealth, gameState, playerVelocityLock, cycleCounter, currentEggs, totalEggs;
extern UINT16 temp16, playerWorldTileStart, temp16b;
extern UBYTE temp1, temp2, temp3, temp4, temp5;
extern struct SPRITE sprites[6];
extern enum SPRITE_DIRECTION playerDirection;
extern UBYTE* currentMap;
extern UBYTE * * * currentMapSprites;

#endif