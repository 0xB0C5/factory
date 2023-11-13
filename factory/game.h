#pragma once

#include <stdint.h>

#define TILE_SIZE 8
#define LEVEL_WIDTH_CELLS 512
#define LEVEL_HEIGHT_CELLS 384
#define PLAYER_INVENTORY_SIZE 16
#define LEVEL_SUBTICKS 9

#define ITEM_NONE 0
#define ITEM_ROCK 1
#define ITEM_COAL 2
#define ITEM_IRON 3
#define ITEM_COPPER 4
#define ITEM_ASSEMBLER 5
#define ITEM_CIRCUIT 6
#define ITEM_COG 7
#define ITEM_CONVEYOR 8
#define ITEM_DRILL 9
// TODO 10
#define ITEM_GRABBER 11
#define ITEM_LAB 12
#define ITEM_IRON_PLATE 13
#define ITEM_COPPER_PLATE 14
#define ITEM_SCIENCE 15
#define ITEM_STORAGE 16
#define ITEM_SWITCHER 17
#define ITEM_WIRE 18
#define ITEM_WOOD 19

#define ITEM_MACHINES_START 20
#define ITEM_FURNACE 20
#define ITEM_FURNACE1 21
#define ITEM_FURNACE2 22
#define ITEM_FURNACE3 23

#define ITEM_ID_COUNT 24

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
  uint32_t tick_counter;
  cell_t level[LEVEL_HEIGHT_CELLS][LEVEL_WIDTH_CELLS];
} game_t;

typedef struct {
  uint8_t item_id;
  uint8_t slot_count;
  uint16_t slot_capacity;
  inventory_item_t items[4];
} machine_inventory_t;

extern game_t game;

void level_init();
bool level_load();
bool level_save();
void level_generate(uint32_t seed);

bool load_machine_inventory(machine_inventory_t *inventory, int16_t x, int16_t y);
bool store_machine_inventory(machine_inventory_t *inventory, int16_t x, int16_t y);

void level_update(uint8_t subtick);
