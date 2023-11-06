#include "level.h"
#include "Arduino.h"

tile_t level[LEVEL_HEIGHT][LEVEL_WIDTH];

void gen_level() {
	for (uint16_t y = 0; y < LEVEL_HEIGHT; y++) {
		for (uint16_t x = 0; x < LEVEL_WIDTH; x++) {
			level[y][x] = random(2);
		}
	}
}

tile_t get_tile(uint16_t x, uint16_t y) {
	return level[y][x];
}
void set_tile(uint16_t x, uint16_t y, tile_t tile) {
	level[y][x] = tile;
}
