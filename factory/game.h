#pragma once

#include <stdint.h>

#define TILE_SIZE 8
#define LEVEL_WIDTH_CELLS 512
#define LEVEL_HEIGHT_CELLS 384
#define PLAYER_INVENTORY_SIZE 16

#define RESOURCE_NONE 0
#define RESOURCE_ROCK 1
#define RESOURCE_COAL 2
#define RESOURCE_IRON 3
#define RESOURCE_COPPER 4
#define RESOURCE_WATER 5
#define RESOURCE_INVALID 6

typedef struct {
  uint8_t id : 6;
  uint16_t data : 10;
} cell_t;

typedef struct {
  uint8_t id;
  uint8_t count;
} inventory_item_t;

typedef struct {
  inventory_item_t inventory[PLAYER_INVENTORY_SIZE];
  uint16_t x;
  uint16_t y;
} player_t;

typedef struct {
  player_t player;
  cell_t level[LEVEL_HEIGHT_CELLS][LEVEL_WIDTH_CELLS];
} game_t;

extern game_t game;

void level_init();
bool level_load();
bool level_save();
void level_generate(uint32_t seed);
