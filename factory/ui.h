#pragma once
#include <stdint.h>

#define UI_STATE_LEVEL 0
#define UI_STATE_INVENTORY 1
#define UI_STATE_CRAFT 2
#define UI_STATE_INFO 3
#define UI_STATE_SETTINGS 4

#define RECIPE_QUEUE_SIZE 256

typedef struct {
	int8_t x;
	int8_t y;
} vec8_t;

// Any state that doesn't survive save & load goes here.
typedef struct {
	uint8_t state;

	uint8_t input;
	uint8_t input_pressed;

	vec8_t player_subcell;
	vec8_t player_velocity;
	vec8_t player_facing;

	uint8_t player_animation_timer;

	uint8_t frame_counter;
	uint8_t action_progress;

  bool menu_tab_selected;
  uint8_t recipe_cursor;
  uint8_t inventory_cursor;

  uint8_t recipe_queue[RECIPE_QUEUE_SIZE];
  int16_t recipe_queue_count;

  uint8_t recipe_progress;
  uint8_t placing_item;
  uint8_t placing_item_size;
  int16_t placing_x;
  int16_t placing_y;
  uint8_t placing_blocked;
} ui_t;

extern ui_t ui;

void ui_reset();
void ui_update();
