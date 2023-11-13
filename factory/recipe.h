#pragma once
#include <stdint.h>
#include "game.h"

typedef struct {
	uint8_t result;
	uint8_t time;
	uint8_t yield;
	inventory_item_t items[4];
} recipe_t;

extern const recipe_t *recipes;
extern const uint16_t recipe_count;
