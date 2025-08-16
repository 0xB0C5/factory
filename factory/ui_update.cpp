#include "ui.h"
#include "input.h"
#include "game.h"
#include "fatal_error.h"
#include "recipe.h"
#include "lcd.h"
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

void add_player_items_if_possible(uint8_t id, uint8_t count) {
  for (int i = 0; i < PLAYER_INVENTORY_SIZE; i++) {
    inventory_item_t *item = &game.player.inventory[i];
    if (item->id == id) {
      int add = count;
      if (item->count + add > 64) {
        add = 64 - item->count;
      }
      
      item->count += add;
      count -= add;
      if (count == 0) return;
    }
  }

  for (int i = 0; i < PLAYER_INVENTORY_SIZE; i++) {
    inventory_item_t *item = &game.player.inventory[i];
    if (item->id == ITEM_NONE) {
      int add = count;
      if (add > 64) {
        add = 64;
      }
      
      item->count = add;
      item->id = id;
      count -= add;
      if (count == 0) return;
    }
  }
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
  if (ui.inventory_cursor >= PLAYER_INVENTORY_SIZE) {
    return;
  }
  uint8_t id = game.player.inventory[ui.inventory_cursor].id;
  switch (id) {
    case ITEM_FURNACE:
    case ITEM_DRILL:
    case ITEM_ASSEMBLER:
    case ITEM_LAB:
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

      ui.placing_data = 0;

      ui.state = UI_STATE_LEVEL;
      break;

    case ITEM_CONVEYOR:
    case ITEM_SWITCHER:
    case ITEM_SPLITTER:
      ui.placing_item = id;
      ui.placing_item_size = 1;
      ui.placing_x = game.player.x + ui.player_facing.x;
      ui.placing_y = game.player.y + ui.player_facing.y;
      ui.placing_data = 0;
      
      ui.state = UI_STATE_LEVEL;
      break;

    default:
      break;
  }
}

void ui_swap_inventory_items() {

  machine_inventory_t machine_inventory;
  bool has_machine = load_machine_inventory(&machine_inventory, game.player.x + ui.player_facing.x, game.player.y + ui.player_facing.y);

  bool uses_machine = false;
  int capacity0 = 64;
  int capacity1 = 64;

  if (ui.inventory_selected >= PLAYER_INVENTORY_SIZE) {
    if (!has_machine) return;
    uses_machine = true;
    capacity0 = machine_inventory.slot_capacity;
  }

  if (ui.inventory_cursor >= PLAYER_INVENTORY_SIZE) {
    if (!has_machine) return;
    uses_machine = true;
    capacity1 = machine_inventory.slot_capacity;
  }

  inventory_item_t *item0 = ui.inventory_selected < PLAYER_INVENTORY_SIZE
    ? &game.player.inventory[ui.inventory_selected]
    : &machine_inventory.items[ui.inventory_selected - PLAYER_INVENTORY_SIZE];
  inventory_item_t *item1 = ui.inventory_cursor < PLAYER_INVENTORY_SIZE
    ? &game.player.inventory[ui.inventory_cursor]
    : &machine_inventory.items[ui.inventory_cursor - PLAYER_INVENTORY_SIZE];

  // Move items.
  inventory_item_t old_item0 = *item0;
  inventory_item_t old_item1 = *item1;

  if (item0->id == ITEM_NONE) {
  
    // Split item1 to item0.
    int n = item1->count / 2;
    if (n > capacity0) n = capacity0;
    item1->count -= n;
    item0->count += n;
    if (item0->count > 0) item0->id = item1->id;

  } else if (item0->id == item1->id || item1->id == ITEM_NONE) {
    // Move item1 to item2.
    int n = item0->count;
    if (item1->count + n > capacity1) {
      n = capacity1 - item1->count;
    }
    item1->count += n;
    if (item1->count > 0) item1->id = item0->id;
    item0->count -= n;
    if (item0->count == 0) item0->id = ITEM_NONE;
  } else {
    // Swap if possible.
    if (item0->count <= capacity1 && item1->count <= capacity0) {
      *item0 = old_item1;
      *item1 = old_item0;
    }
  }

  if (uses_machine) {
    if (!store_machine_inventory(&machine_inventory, game.player.x + ui.player_facing.x, game.player.y + ui.player_facing.y)) {
      // Undo.
      *item0 = old_item0;
      *item1 = old_item1;
      return;
    }
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
      {
        machine_inventory_t machine_inventory;
        bool has_machine = load_machine_inventory(&machine_inventory, game.player.x + ui.player_facing.x, game.player.y + ui.player_facing.y);
        int machine_slots = has_machine ? machine_inventory.slot_count : 0;
        int total_slots = PLAYER_INVENTORY_SIZE + machine_slots;

        if (ui.inventory_cursor >= total_slots) {
          ui.inventory_cursor = 0;
        }

        if (ui.inventory_selected != 0xff && ui.inventory_selected >= total_slots) {
          ui.inventory_selected = 0;
        }

        // If we press A, select the item.
        if (ui.input_pressed & INPUT_A) {
          ui.inventory_selected = ui.inventory_cursor;
        }

        // Move the cursor.
        uint8_t prev_cursor = ui.inventory_cursor;
        if (ui.input_pressed & INPUT_RIGHT) {
          ui.inventory_cursor++;

          if (ui.inventory_cursor >= total_slots) {
            ui.inventory_cursor = 0;
          }
        }
        if (ui.input_pressed & INPUT_LEFT) {
          if (ui.inventory_cursor == 0) {
            ui.inventory_cursor = total_slots;
          }
          ui.inventory_cursor--;
        }
        if (ui.input_pressed & INPUT_DOWN) {
          if (ui.inventory_cursor >= PLAYER_INVENTORY_SIZE) {
            ui.inventory_cursor -= PLAYER_INVENTORY_SIZE;
            ui.inventory_cursor *= 2;
          } else if (ui.inventory_cursor < 8) {
            ui.inventory_cursor += 8;
          } else if (has_machine) {
            ui.inventory_cursor = PLAYER_INVENTORY_SIZE + (ui.inventory_cursor - 8) / 2;
            if (ui.inventory_cursor >= total_slots) ui.inventory_cursor = total_slots-1;
          } else {
            ui.inventory_cursor -= 8;
          }
        }
        if (ui.input_pressed & INPUT_UP) {
          if (ui.inventory_cursor >= PLAYER_INVENTORY_SIZE) {
            ui.inventory_cursor -= PLAYER_INVENTORY_SIZE;
            ui.inventory_cursor *= 2;
            ui.inventory_cursor += 8;
          } else if (ui.inventory_cursor >= 8) {
            ui.inventory_cursor -= 8;
          } else if (has_machine) {
            ui.inventory_cursor = PLAYER_INVENTORY_SIZE + ui.inventory_cursor / 2;
            if (ui.inventory_cursor >= total_slots) ui.inventory_cursor = total_slots-1;
          } else {
            ui.inventory_cursor += 8;
          }
        }
  
        // If we drag, flag we are swapping.
        if ((ui.input & INPUT_A) && ui.inventory_cursor != prev_cursor) {
          ui.inventory_swapping = true;
        }
  
        if (ui.input_released & INPUT_A && ui.inventory_selected != 0xff) {
          if (ui.inventory_swapping) {
            ui_swap_inventory_items();
          } else {
            ui_select_inventory_item();
          }
          ui.inventory_selected = 0xff;
          ui.inventory_swapping = false;
        }
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
      if (ui.input_pressed & (INPUT_DOWN | INPUT_UP)) {
        if (ui.recipe_cursor < 8) {
          if (ui.recipe_cursor + 8 < recipe_count) {
            ui.recipe_cursor += 8;
          } else {
            ui.recipe_cursor = recipe_count;
          }
        } else {
          if (ui.recipe_cursor == recipe_count) {
            ui.recipe_cursor = 7;
          } else {
            ui.recipe_cursor -= 8;
          }
        }
      }

      if (ui.input_pressed & INPUT_A) {
        int16_t facing_y = game.player.y + ui.player_facing.y;
        int16_t facing_x = game.player.x + ui.player_facing.x;
        uint8_t facing_id = game.level[facing_y][facing_x].id;
        if (facing_id >= ITEM_ASSEMBLER && facing_id <= ITEM_ASSEMBLER3) {
          // Set the assembler's recipe.
          int8_t diff = facing_id - ITEM_ASSEMBLER;
          int8_t dx = diff & 1;
          int8_t dy = diff >> 1;
          int16_t assembler_x = facing_x - dx;
          int16_t assembler_y = facing_y - dy;
          cell_t *data_cell = &game.level[assembler_y+1][assembler_x+1];
          uint16_t recipe_data;
          if (ui.recipe_cursor < recipe_count) {
            recipe_data = ui.recipe_cursor + 1;
          } else {
            recipe_data = 0;
          }
          data_cell->data = (data_cell->data & ~0b11111) | recipe_data;
        } else {
          // Craft.
          if (ui.recipe_cursor == recipe_count) {
            // Cancel recipes.
            ui.recipe_queue_count = 0;
            ui.recipe_progress = 0;
          } else {
            ui_update_queue_selected_recipe();
          }
        }
      }
      break;

    case UI_STATE_SETTINGS:
      if (ui.input_pressed & (INPUT_DOWN | INPUT_UP)) {
        if (ui.settings_cursor < 3) {
          ui.settings_cursor = 3;
        } else {
          ui.settings_cursor = 0;
        }
      }
    
      if (ui.settings_cursor < 3) {
        if (ui.input_pressed & INPUT_RIGHT) {
          ui.settings_cursor++;
          if (ui.settings_cursor >= 3) {
            ui.settings_cursor = 0;
          }
        }
        if (ui.input_pressed & INPUT_LEFT) {
          if (ui.settings_cursor == 0) {
            ui.settings_cursor = 2;
          } else {
            ui.settings_cursor--;
          }
        }
  
        if (ui.input_pressed & INPUT_A) {
          switch (ui.settings_cursor) {
            case 0:
              ui.save_requested = true;
              break;
            case 1:
              ui.delete_requested = true;
              break;
            case 2:
              game.autosave = !game.autosave;
          }
        }
      } else {
        int contrast = lcd_get_contrast();
        int new_contrast = contrast;
        if (ui.input_pressed & INPUT_RIGHT) {
          new_contrast += 2;
          if (new_contrast > 127) new_contrast = 127;
        }

        if (ui.input_pressed & INPUT_LEFT) {
          new_contrast -= 2;
          if (new_contrast < 0) new_contrast = 0;
        }

        if (new_contrast != contrast) {
          game.contrast = new_contrast;
          lcd_set_contrast(new_contrast);
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
      
      case ITEM_ASSEMBLER:
      case ITEM_ASSEMBLER1:
      case ITEM_ASSEMBLER2:
      case ITEM_ASSEMBLER3:
      
      case ITEM_LAB:
      case ITEM_LAB1:
      case ITEM_LAB2:
      case ITEM_LAB3:
      
      case ITEM_DRILL:
      case ITEM_DRILL1:
      case ITEM_DRILL2:
      case ITEM_DRILL3:
        {
          uint8_t offset = cell->id % 4;
          uint8_t base_item = cell->id - offset;
  
          int16_t item_pos_y = action_y - (offset >> 1);
          int16_t item_pos_x = action_x - (offset & 1);
  
          if (ui_try_add_player_inventory(base_item)) {
            machine_inventory_t machine_inventory;
            if (load_machine_inventory(&machine_inventory, action_x, action_y)) {
              for (int i = 0; i < machine_inventory.slot_count; i++) {
                inventory_item_t item = machine_inventory.items[i];
                if (item.id != ITEM_NONE) {
                  add_player_items_if_possible(item.id, item.count);
                }
              }
            }

            
            cell_t blank = {0, 0};
            game.level[item_pos_y][item_pos_x] = blank;
            game.level[item_pos_y][item_pos_x+1] = blank;
            game.level[item_pos_y+1][item_pos_x] = blank;
            game.level[item_pos_y+1][item_pos_x+1] = blank;
          }
          ui.action_progress = 0;
        }
        break;

      case ITEM_CONVEYOR:
      case ITEM_SWITCHER:
      case ITEM_SPLITTER:
        {
          if (ui_try_add_player_inventory(cell->id)) {
            uint8_t item_id = (cell->data >> 2) & 0b11111;
            if (item_id != ITEM_NONE) {
              ui_try_add_player_inventory(item_id);
            }
            
            cell_t blank = {0, 0};
            *cell = blank;
          }
          ui.action_progress = 0;
        }
        break;

      default:
        // Not an actionable tile.
        ui.action_progress = 0;
        return;
    }
  }
}

void ui_update_level() {
  bool is_player_aligned = ui.player_subcell.x == 0 && ui.player_subcell.y == 0;
  if (is_player_aligned && ui.input_pressed & INPUT_B) {
    if (ui.placing_item != ITEM_NONE) {
      ui.placing_item = ITEM_NONE;
    } else {
      ui.state = UI_STATE_INVENTORY;
      ui.menu_tab_selected = false;
    }
    return;
  }

  if (ui.placing_item != ITEM_NONE) {
    if ((ui.input & INPUT_A) && ui.placing_item == ITEM_CONVEYOR) {
      if (ui.input_pressed & INPUT_UP) {
        ui.placing_data = DIRECTION_UP;
      }
      if (ui.input_pressed & INPUT_DOWN) {
        ui.placing_data = DIRECTION_DOWN;
      }
      if (ui.input_pressed & INPUT_LEFT) {
        ui.placing_data = DIRECTION_LEFT;
      }
      if (ui.input_pressed & INPUT_RIGHT) {
        ui.placing_data = DIRECTION_RIGHT;
      }
    } else {
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

    if (ui.placing_blocked == 0 && ui.input_released & INPUT_A) {
      uint8_t id = ui.placing_item;

      inventory_item_t *item = &game.player.inventory[ui.inventory_cursor];
      if (item->id != id || item->count == 0) return;
      item->count--;
      if (item->count == 0) {
        item->id = 0;
        ui.placing_item = ITEM_NONE;
      }

      // Place the item!
      if (ui.placing_item_size == 2) {
        cell_t cell0 = {id, 0};
        cell_t cell1 = {(uint8_t)(id+1), 0};
        cell_t cell2 = {(uint8_t)(id+2), 0};
        cell_t cell3 = {(uint8_t)(id+3), 0};
  
        if (id == ITEM_DRILL) {
          uint8_t resource_id = ITEM_NONE;
          int16_t resource_count = 0;
          // Set the drill's resource count.
          for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
              cell_t existing_cell = game.level[ui.placing_y+dy][ui.placing_x+dx];
              switch (existing_cell.id) {
                case ITEM_COAL:
                case ITEM_ROCK:
                case ITEM_IRON:
                case ITEM_COPPER:
                  resource_id = existing_cell.id;
                  resource_count += existing_cell.data;
                  break;
                default:
                  break;
              }
            }
          }
  
          if (resource_count > 2047) {
            resource_count = 2047;
          }
  
          cell2.data = resource_id;
          cell3.data = resource_count;
        }
  
        game.level[ui.placing_y][ui.placing_x] = cell0;
        game.level[ui.placing_y][ui.placing_x+1] = cell1;
        game.level[ui.placing_y+1][ui.placing_x] = cell2;
        game.level[ui.placing_y+1][ui.placing_x+1] = cell3;
      } else {
        // size = 1
        cell_t cell = {id, ui.placing_data};
        game.level[ui.placing_y][ui.placing_x] = cell;
      }
    }
    return;
  }

  // Update player velocity.
  if (is_player_aligned) {
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

  // Check collision.
  if (is_player_aligned && (ui.player_velocity.x != 0 || ui.player_velocity.y != 0)) {
    int16_t x = game.player.x + ui.player_velocity.x;
    int16_t y = game.player.y + ui.player_velocity.y;

    switch (game.level[y][x].id) {
      case ITEM_NONE:
      case ITEM_COAL:
      case ITEM_ROCK:
      case ITEM_IRON:
      case ITEM_COPPER:
      case ITEM_CONVEYOR:
      case ITEM_SPLITTER:
      case ITEM_SWITCHER:
        break;
      default:
        ui.player_velocity.x = 0;
        ui.player_velocity.y = 0;
        break;
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

  // Update action progress.
  ui_update_action();
}

void craft_recipe(const recipe_t *recipe) {
  // Check we have enough items.
  for (int i = 0; i < 4; i++) {
    inventory_item_t recipe_item = recipe->items[i];
    if (recipe_item.id == ITEM_NONE) break;
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
    if (recipe_item.id == ITEM_NONE) break;
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
  if (!ui.save_requested) {
    // Update input.
    uint8_t input = input_read();
    uint8_t pressed = input & (~ui.input);
    uint8_t released = ui.input & (~input);
    ui.input = input;
    ui.input_pressed = pressed;
    ui.input_released = released;
  
    if (ui.state == UI_STATE_LEVEL) {
        ui_update_level();
    } else {
        ui_update_menu();
    }
  
    // Clear screen-dependent state.
    if (ui.state == UI_STATE_LEVEL) {
      ui.menu_tab_selected = false;
    } else {
      ui.placing_item = ITEM_NONE;
    }
  
    if (!ui.menu_tab_selected) {
      ui.inventory_selected = 0xff;
      ui.inventory_swapping = false;
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

    // Autosave, approx every 5 minutes.
    if (game.autosave && game.tick_counter - game.save_tick >= 300) {
      ui.save_requested = true;
    }
  }

  ui.frame_counter++;
  ui.level_subtick = (ui.level_subtick + 1) % LEVEL_SUBTICKS;
}
