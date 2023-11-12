#include "ui.h"
#include "input.h"
#include "game.h"
#include "fatal_error.h"

bool ui_try_add_player_inventory(uint8_t id) {
  for (int i = 0; i < PLAYER_INVENTORY_SIZE; i++) {
    inventory_item_t *item = &game.player.inventory[i];
    if (item->id == id && item->count < 64) {
      item->count++;
      return true;
    }
  }

  for (int i = 0; i < PLAYER_INVENTORY_SIZE; i++) {
    inventory_item_t *item = &game.player.inventory[i];
    if (item->id == ITEM_NONE) {
      item->id = id;
      item->count = 1;
      return true;
    }
  }

  return false;
}

void ui_update_menu() {
  if (ui.input_pressed & INPUT_B) {
    ui.state = UI_STATE_LEVEL;
    return;
  }
}

void ui_update_action() {
  // If you start moving, cancel the action.
  if (ui.player_velocity.x != 0 || ui.player_velocity.y != 0) {
    ui.action_progress = 0;
    return;
  }

  // If you stop holding A, cancel the action.
  if ((ui.input & INPUT_A) == 0) {
    ui.action_progress = 0;
    return;
  }
  
  int16_t action_x = game.player.x + ui.player_facing.x;
  int16_t action_y = game.player.y + ui.player_facing.y;

  if (ui.action_progress == 0) {
    // Check if we can start an action.
    cell_t cell = game.level[action_y][action_x];
    switch (cell.id) {
      case ITEM_ROCK:
      case ITEM_COAL:
      case ITEM_IRON:
      case ITEM_COPPER:
        break;

      default:
        // Not an actionable cell.
        return;
    }

    // Begin action.
    ui.action_progress = 1;
    return;
  }

  // An action is in progress.
  ui.action_progress++;

  if (ui.action_progress == 13) {
    ui.action_progress = 0;

    // Do the action!
    cell_t *cell = &game.level[action_y][action_x];
    switch (cell->id) {
      case ITEM_ROCK:
      case ITEM_COAL:
      case ITEM_IRON:
      case ITEM_COPPER:
        if (ui_try_add_player_inventory(cell->id)) {
          if (--cell->data == 0) {
            cell->id = ITEM_NONE;
          }
        } else {
          // TODO : show action failed sprite.
        }

        break;

      default:
        // Not an actionable tile.
        return;
    }
  }
}

void ui_update_level() {
  // Update player velocity.
  bool is_player_aligned = ui.player_subcell.x == 0 && ui.player_subcell.y == 0;
  if (is_player_aligned) {
    if (ui.input_pressed & INPUT_B) {
      ui.state = UI_STATE_MENU;
      return;
    }

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

  // Update animation and facing.
  if (ui.player_velocity.x != 0 || ui.player_velocity.y != 0) {
    ui.player_animation_timer++;
    ui.player_facing = ui.player_velocity;
  } else {
    ui.player_animation_timer = 0;
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

  // Update action progress.
  ui_update_action();

  ui.frame_counter++;
}

void ui_update() {
  // Update input.
  uint8_t input = input_read();
  uint8_t pressed = input & (~ui.input);
  ui.input = input;
  ui.input_pressed = pressed;

  switch (ui.state) {
    case UI_STATE_LEVEL:
      ui_update_level();
      break;
    case UI_STATE_MENU:
      ui_update_menu();
      break;
      
  }
}
