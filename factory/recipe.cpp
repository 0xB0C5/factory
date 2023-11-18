#include "recipe.h"
#include "game.h"

static const recipe_t recipe_array[] = {
  {
    ITEM_COG, 16, 1, {{ITEM_IRON_PLATE, 2}},
  },
  {
    ITEM_WIRE, 16, 2, {{ITEM_COPPER_PLATE, 1}},
  },
	{
		ITEM_CIRCUIT, 16, 1, {{ITEM_WIRE, 3}, {ITEM_IRON_PLATE, 1}},
	},
  {
    ITEM_SCIENCE, 32, 1, {{ITEM_COPPER_PLATE, 1}, {ITEM_COG, 1}},
  },
  {
    ITEM_CONVEYOR, 16, 2, {{ITEM_COG, 1}, {ITEM_IRON_PLATE, 1}},
  },
  {
    ITEM_FURNACE, 32, 1, {{ITEM_ROCK, 5}},
  },
  {
    ITEM_DRILL, 32, 1, {{ITEM_COG, 3}, {ITEM_IRON_PLATE, 3}, {ITEM_FURNACE, 1}},
  },
  /*
  {
    ITEM_ASSEMBLER, 64, 1, {{ITEM_CIRCUIT, 3}, {ITEM_COG, 5}, {ITEM_IRON_PLATE, 9}},
  },
  */
  {
    ITEM_LAB, 64, 1, {{ITEM_CIRCUIT, 10}, {ITEM_COG, 10}, {ITEM_CONVEYOR, 4}},
  },
};

const recipe_t *recipes = recipe_array;
const uint16_t recipe_count = sizeof(recipe_array) / sizeof(recipe_t);
