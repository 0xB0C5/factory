#include "ui.h"
#include "input.h"
#include "game.h"
#include "fatal_error.h"
#include "recipe.h"
#include <string.h>

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

static int16_t temp_item_counts[ITEM_ID_COUNT];

void ui_update_queue_selected_recipe() {
  if (ui.recipe_queue_count == RECIPE_QUEUE_SIZE) return;

  // Compute total count of all items.
  memset(&temp_item_counts, 0, sizeof(temp_item_counts));
  for (int i = 0; i < PLAYER_INVENTORY_SIZE; i++) {
    inventory_item_t item = game.player.inventory[i];
    temp_item_counts[item.id] += item.count;
  }
  // Apply the queued recipes to the counts.
  for (int i = 0; i < ui.recipe_queue_count; i++) {
    const recipe_t *recipe = &recipes[ui.recipe_queue[i]];
  
    for (int j = 0; j < 4; j++) {
      inventory_item_t item = recipe->items[j];
      if (item.id == ITEM_NONE) break;
      temp_item_counts[item.id] -= item.count;
    }
  
    temp_item_counts[recipe->result] += recipe->yield;
  }
  
  // Check if items are available for the recipe.
  const recipe_t *recipe = &recipes[ui.recipe_cursor];
  bool has_items = true;
  for (int i = 0; i < 4; i++) {
    inventory_item_t item = recipe->items[i];
    temp_item_counts[item.id] -= item.count;
    if (temp_item_counts[item.id] < 0) {
      has_items = false;
      break;
    }
  }
  
  if (!has_items) return;

  // Queue the item!
  ui.recipe_queue[ui.recipe_queue_count++] = ui.recipe_cursor;
}

void ui_select_inventory_item() {
  uint8_t id = game.player.inventory[ui.inventory_cursor].id;
  switch (id) {
    case ITEM_FURNACE:
      // Start placing 2x2 item.
      ui.placing_item = id;
      ui.placing_item_size = 2;

      ui.placing_x = game.player.x + ui.player_facing.x;
      ui.placing_y = game.player.y + ui.player_facing.y;

      if (ui.player_facing.x < 0) {
        ui.placing_x--;
      }

      if (ui.player_facing.y < 0) {
        ui.placing_y--;
      }

      ui.state = UI_STATE_LEVEL;
      break;
    default:
      break;
  }
}

void ui_update_menu() {
  if (!ui.menu_tab_selected) {
    if (ui.input_pressed & INPUT_B) {
      ui.state = UI_STATE_LEVEL;
      return;
    }

    if (ui.input_pressed & INPUT_RIGHT) {
      ui.state++;
      if (ui.state > UI_STATE_SETTINGS) {
        ui.state = UI_STATE_INVENTORY;
      }
    }
    if (ui.input_pressed & INPUT_LEFT) {
      ui.state--;
      if (ui.state < UI_STATE_INVENTORY) {
        ui.state = UI_STATE_SETTINGS;
      }
    }
    
    if (ui.input_pressed & INPUT_A) {
      ui.menu_tab_selected = true;
    }
    return;
  }

  if (ui.input_pressed & INPUT_B) {
    ui.menu_tab_selected = false;
    return;
  }

  switch (ui.state) {
    case UI_STATE_INVENTORY:
      if (ui.input_pressed & INPUT_RIGHT) {
        ui.inventory_cursor++;
      }
      if (ui.input_pressed & INPUT_LEFT) {
        ui.inventory_cursor--;
      }
      if (ui.input_pressed & INPUT_DOWN) {
        ui.inventory_cursor += 8;
      }
      if (ui.input_pressed & INPUT_UP) {
        ui.inventory_cursor -= 8;
      }
      ui.inventory_cursor %= PLAYER_INVENTORY_SIZE;

      if (ui.input_pressed & INPUT_A) {
        ui_select_inventory_item();
      }

      break;

    case UI_STATE_CRAFT:
      if (ui.input_pressed & INPUT_RIGHT) {
        ui.recipe_cursor++;
        if (ui.recipe_cursor > recipe_count) {
          ui.recipe_cursor = 0;
        }
      }
      if (ui.input_pressed & INPUT_LEFT) {
        ui.recipe_cursor--;
        if (ui.recipe_cursor > recipe_count) {
          ui.recipe_cursor = recipe_count;
        }
      }
      if (ui.input_pressed & INPUT_DOWN) {
        ui.recipe_cursor += 8;
        if (ui.recipe_cursor > recipe_count) {
          ui.recipe_cursor %= 8;
        }
      }
      if (ui.input_pressed & INPUT_UP) {
        // There's probably a better way to do this but whatever.
        ui.recipe_cursor -= 8;
        if (ui.recipe_cursor > recipe_count) {
          do {
            ui.recipe_cursor += 8;
          } while (ui.recipe_cursor <= recipe_count);

          ui.recipe_cursor -= 8;
        }
      }
      if (ui.input_pressed & INPUT_A) {
        if (ui.recipe_cursor == recipe_count) {
          // Cancel recipes.
          ui.recipe_queue_count = 0;
          ui.recipe_progress = 0;
        } else {
          ui_update_queue_selected_recipe();
        }
      }
      break;

    default:
      break;
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
    if ((ui.input_pressed & INPUT_A) == 0) return;
    
    // Check if we can start an action.
    cell_t cell = game.level[action_y][action_x];
    if (cell.id == ITEM_NONE) return;

    // Begin action.
    ui.action_progress = 1;
    return;
  }

  // An action is in progress.
  ui.action_progress++;

  if (ui.action_progress == 13) {
    ui.action_progress = 1;

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
            ui.action_progress = 0;
          }
        } else {
          // TODO : show action failed sprite.
        }

        break;

      case ITEM_FURNACE:
      case ITEM_FURNACE1:
      case ITEM_FURNACE2:
      case ITEM_FURNACE3:
        {
          uint8_t offset = cell->id % 4;
          uint8_t base_item = cell->id - offset;
  
          int16_t item_pos_y = action_y - (offset >> 1);
          int16_t item_pos_x = action_x - (offset & 1);
  
          if (ui_try_add_player_inventory(base_item)) {
            cell_t blank = {0, 0};
            game.level[item_pos_y][item_pos_x] = blank;
            game.level[item_pos_y][item_pos_x+1] = blank;
            game.level[item_pos_y+1][item_pos_x] = blank;
            game.level[item_pos_y+1][item_pos_x+1] = blank;
          }
          ui.action_progress = 0;
        }

      default:
        // Not an actionable tile.
        ui.action_progress = 0;
        return;
    }
  }
}

void ui_update_level() {
  if (ui.placing_item != ITEM_NONE) {
    if (ui.input_pressed & INPUT_UP) {
      ui.placing_y--;
    }
    if (ui.input_pressed & INPUT_DOWN) {
      ui.placing_y++;
    }
    if (ui.input_pressed & INPUT_LEFT) {
      ui.placing_x--;
    }
    if (ui.input_pressed & INPUT_RIGHT) {
      ui.placing_x++;
    }
    uint16_t max_placing_x = game.player.x + 5 - ui.placing_item_size;
    uint16_t min_placing_x = game.player.x - 4;
    if (ui.placing_x > max_placing_x) {
      ui.placing_x = max_placing_x;
    } else if (ui.placing_x < min_placing_x) {
      ui.placing_x = min_placing_x;
    }

    uint16_t max_placing_y = game.player.y + 4 - ui.placing_item_size;
    uint16_t min_placing_y = game.player.y - 3;
    if (ui.placing_y > max_placing_y) {
      ui.placing_y = max_placing_y;
    } else if (ui.placing_y < min_placing_y) {
      ui.placing_y = min_placing_y;
    }

    // Compute obstacles
    ui.placing_blocked = 0;
    int obstacle_index = 0;
    for (int dy = 0; dy < ui.placing_item_size; dy++) {
      for (int dx = 0; dx < ui.placing_item_size; dx++) {
        bool is_blocked;
        switch (game.level[ui.placing_y+dy][ui.placing_x+dx].id) {
          case ITEM_NONE:
          case ITEM_ROCK:
          case ITEM_COAL:
          case ITEM_IRON:
          case ITEM_COPPER:
            is_blocked = ui.placing_x+dx == game.player.x && ui.placing_y+dy == game.player.y;
            break;
          default:
            is_blocked = true;
        }
        if (is_blocked) {
            ui.placing_blocked |= 1 << obstacle_index;
        }
        obstacle_index++;
      }
    }

    if (ui.placing_blocked == 0 && ui.input_pressed & INPUT_A) {
      uint8_t id = ui.placing_item;
      ui.placing_item = ITEM_NONE;

      inventory_item_t *item = &game.player.inventory[ui.inventory_cursor];
      if (item->id != id || item->count == 0) return;
      item->count--;
      if (item->count == 0) {
        item->id = 0;
      }

      // Place the item!
      cell_t cell = {id, 0};
      game.level[ui.placing_y][ui.placing_x] = cell;
      cell.id = id + 1;
      game.level[ui.placing_y][ui.placing_x+1] = cell;
      cell.id = id + 2;
      game.level[ui.placing_y+1][ui.placing_x] = cell;
      cell.id = id + 3;
      game.level[ui.placing_y+1][ui.placing_x+1] = cell;
    }
    return;
  }

  // Update player velocity.
  bool is_player_aligned = ui.player_subcell.x == 0 && ui.player_subcell.y == 0;
  if (is_player_aligned) {
    if (ui.input_pressed & INPUT_B) {
      ui.state = UI_STATE_INVENTORY;
      ui.placing_item = ITEM_NONE;
      ui.menu_tab_selected = false;
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

    if (ui.player_velocity.y == 0) {
      if (ui.input & INPUT_LEFT) {
        ui.player_velocity.x -= 1;
      }
  
      if (ui.input & INPUT_RIGHT) {
        ui.player_velocity.x += 1;
      }
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
}

void craft_recipe(const recipe_t *recipe) {
  // Check we have enough items.
  for (int i = 0; i < 4; i++) {
    inventory_item_t recipe_item = recipe->items[i];
    int count = 0;
    for (int j = 0; j < PLAYER_INVENTORY_SIZE; j++) {
      inventory_item_t player_item = game.player.inventory[j];
      if (player_item.id == recipe_item.id) {
        count += player_item.count;
        if (count >= recipe_item.count) break;
      }
    }
    if (count < recipe_item.count) return;
  }

  // Find a slot we can put the items in.
  int slot = -1;
  for (int i = 0; i < PLAYER_INVENTORY_SIZE; i++) {
    inventory_item_t player_item = game.player.inventory[i];
    if (player_item.id == ITEM_NONE || (player_item.id == recipe->result && player_item.count + recipe->yield <= 64)) {
      slot = i;
      break;
    }
  }

  if (slot == -1) return;

  // Remove the items.
  for (int i = 0; i < 4; i++) {
    inventory_item_t recipe_item = recipe->items[i];
    int remaining = recipe_item.count;
    for (int j = PLAYER_INVENTORY_SIZE-1; j >= 0; j--) {
      inventory_item_t *player_item = &game.player.inventory[j];
      if (player_item->id == recipe_item.id) {
        if (player_item->count > remaining) {
          player_item->count -= remaining;
          remaining = 0;
        } else {
          remaining -= player_item->count;
          player_item->count = 0;
          player_item->id = ITEM_NONE;
        }
        if (remaining == 0) break;
      }
    }
  }

  // Add the item.
  inventory_item_t *new_item = &game.player.inventory[slot];
  new_item->id = recipe->result;
  new_item->count += recipe->yield;
}

void ui_update() {
  // Update input.
  uint8_t input = input_read();
  uint8_t pressed = input & (~ui.input);
  ui.input = input;
  ui.input_pressed = pressed;

  if (ui.state == UI_STATE_LEVEL) {
      ui_update_level();
  } else {
      ui_update_menu();
  }

  // Update crafting.
  if (ui.recipe_queue_count) {
    const recipe_t *recipe = &recipes[ui.recipe_queue[0]];
    if (ui.recipe_progress < recipe->time) {
      ui.recipe_progress++;
    } else {
      ui.recipe_progress = 0;
      craft_recipe(recipe);
      // Pop the 0th recipe from the queue.
      ui.recipe_queue_count--;
      for (int i = 0; i < ui.recipe_queue_count; i++) {
        ui.recipe_queue[i] = ui.recipe_queue[i+1];
      }
    }
  }

  ui.frame_counter++;
}
