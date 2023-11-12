#pragma once
#include <stdint.h>

#define UI_STATE_GAMEPLAY 0
#define UI_STATE_MENU 1

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
} ui_t;

extern ui_t ui;

void ui_reset();
void ui_update();
