#include <stdint.h>

typedef uint8_t tile_t;

#define TILE_GROUND 1
#define TILE_WATER 2
#define TILE_TREE 3
#define TILE_STONE 4
#define TILE_IRON 5
#define TILE_COPPER 6
#define TILE_COAL 7

#define TILE_CONVEYOR_UP 8
#define TILE_CONVEYOR_DOWN 9
#define TILE_CONVEYOR_LEFT 10
#define TILE_CONVEYOR_RIGHT 11

#define LEVEL_WIDTH 512
#define LEVEL_HEIGHT 512

void gen_level();

tile_t get_tile(uint16_t x, uint16_t y);
void set_tile(uint16_t x, uint16_t y, tile_t tile);
