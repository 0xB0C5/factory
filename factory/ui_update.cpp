#include "ui.h"
#include "input.h"
#include "game.h"

void ui_update() {
  // Update input.
  uint8_t input = input_read();
  uint8_t pressed = input & (~ui.input);
  ui.input = input;
  ui.input_pressed = pressed;

  // Update player velocity.
  bool is_player_aligned = ui.player_subcell.x == 0 && ui.player_subcell.y == 0;
  if (is_player_aligned) {
    ui.player_velocity.x = 0;
    ui.player_velocity.y = 0;
    if (ui.input & INPUT_UP) {
      ui.player_velocity.y -= 1;
    }

    if (ui.input & INPUT_DOWN) {
      ui.player_velocity.y += 1;
    }

    if (ui.input & INPUT_LEFT) {
      ui.player_velocity.x -= 1;
    }

    if (ui.input & INPUT_RIGHT) {
      ui.player_velocity.x += 1;
    }
  }

  // Move player
  ui.player_subcell.x += ui.player_velocity.x;
  ui.player_subcell.y += ui.player_velocity.y;

  if (ui.player_subcell.x >= 4) {
    ui.player_subcell.x -= 4;
    game.player.x++;
  } else if (ui.player_subcell.x <= -4) {
    ui.player_subcell.x += 4;
    game.player.x--;
  }
  
  if (ui.player_subcell.y >= 4) {
    ui.player_subcell.y -= 4;
    game.player.y++;
  } else if (ui.player_subcell.y <= -4) {
    ui.player_subcell.y += 4;
    game.player.y--;
  }

  // Update animation and facing.
  if (ui.player_velocity.x != 0 || ui.player_velocity.y != 0) {
    ui.player_animation_timer++;
    ui.player_facing = ui.player_velocity;
  } else {
    ui.player_animation_timer = 0;
  }

  // Update action progress.
  if (is_player_aligned && ((ui.input & INPUT_A) != 0) && ui.action_progress < 12) {
    ui.action_progress++;
  } else {
    ui.action_progress = 0;
  }

  ui.frame_counter++;
}