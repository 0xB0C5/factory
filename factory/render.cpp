#include "render.h"
#include "ui.h"
#include "lcd.h"
#include "game.h"
#include "generated_graphics.h"
#include "recipe.h"

#define CAMERA_MAX_X (8*LEVEL_WIDTH_CELLS-LCD_WIDTH)
#define CAMERA_MAX_Y (7*LEVEL_HEIGHT_CELLS-LCD_HEIGHT)


void render_item(uint8_t id, int8_t x, int8_t y) {
  const uint8_t *sprite;
  switch (id) {
    case ITEM_ROCK:
      sprite = sprite_rock[0];
      break;
    case ITEM_COAL:
      sprite = sprite_coal[0];
      break;
    case ITEM_IRON:
      sprite = sprite_iron_ore[0];
      break;
    case ITEM_COPPER:
      sprite = sprite_copper_ore[0];
      break;
    case ITEM_ASSEMBLER:
      sprite = sprite_assembler_icon[0];
      break;
    case ITEM_CIRCUIT:
      sprite = sprite_circuit[0];
      break;
    case ITEM_COG:
      sprite = sprite_cog[0];
      break;
    case ITEM_CONVEYOR:
      sprite = sprite_conveyor_icon[0];
      break;
    case ITEM_DRILL:
      sprite = sprite_drill_icon[0];
      break;
    case ITEM_FURNACE:
      sprite = sprite_furnace_icon[0];
      break;
    case ITEM_GRABBER:
      sprite = sprite_grabber_icon[0];
      break;
    case ITEM_LAB:
      sprite = sprite_lab_icon[0];
      break;
    case ITEM_IRON_PLATE:
      sprite = sprite_iron_plate[0];
      break;
    case ITEM_COPPER_PLATE:
      sprite = sprite_copper_plate[0];
      break;
    case ITEM_SCIENCE:
      sprite = sprite_science[0];
      break;
    case ITEM_SWITCHER:
      sprite = sprite_switcher_icon[0];
      break;
    case ITEM_WIRE:
      sprite = sprite_wire[0];
      break;
      /*
    case ITEM_STORAGE:
      sprite = sprite_storage_icon[0];
      break;
    case ITEM_WOOD:
      sprite = sprite_wood[0];
      break;
      */

    default:
      // Don't render.
      return;
  }

  lcd_draw_sprite(sprite, x, y);
}

void render_2_digits(uint8_t value, int8_t x, int8_t y) {
  lcd_draw_digit(value / 10, x, y);
  lcd_draw_digit(value % 10, x + 4, y);
}

void render_count(uint8_t count, int8_t x, int8_t y) {
  if (count >= 10) {
    lcd_draw_digit(count / 10, x, y);
    lcd_draw_digit(count % 10, x + 4, y);
  } else {
    lcd_draw_digit(count, x + 2, y);
  }
}

void render_cursor(int8_t x, int8_t y) {
  int cursor_index = ((ui.frame_counter >> 1) & 3) << 2;
  lcd_draw_sprite(sprite_cursor[cursor_index], x-1, y-1);
  lcd_draw_sprite(sprite_cursor[cursor_index|1], x+7, y-1);
  lcd_draw_sprite(sprite_cursor[cursor_index|2], x-1, y+7);
  lcd_draw_sprite(sprite_cursor[cursor_index|3], x+7, y+7);
}

void render_menu() {
  lcd_clear();

  uint8_t visible_state = ui.state;
  if (!ui.menu_tab_selected && (ui.frame_counter & 0b10)) {
    visible_state = 0xff;
  }

  // Render menu tabs.
  // inventory tab
  lcd_draw_bg(bg_cell_menu_tab[visible_state == UI_STATE_INVENTORY ? 1 : 0], 24, 0);
  lcd_draw_bg(bg_cell_menu_tab[visible_state == UI_STATE_INVENTORY ? 9 : 8], 32, 0);
  // craft tab
  lcd_draw_bg(bg_cell_menu_tab[visible_state == UI_STATE_CRAFT ? 3 : 2], 35, 0);
  lcd_draw_bg(bg_cell_menu_tab[visible_state == UI_STATE_CRAFT ? 9 : 8], 43, 0);
  // info tab
  lcd_draw_bg(bg_cell_menu_tab[visible_state == UI_STATE_INFO ? 5 : 4], 46, 0);
  lcd_draw_bg(bg_cell_menu_tab[visible_state == UI_STATE_INFO ? 9 : 8], 54, 0);
  // settings tab
  lcd_draw_bg(bg_cell_menu_tab[visible_state == UI_STATE_SETTINGS ? 7 : 6], 57, 0);
  lcd_draw_bg(bg_cell_menu_tab[visible_state == UI_STATE_SETTINGS ? 11 : 10], 65, 0);

  switch (ui.state) {
    case UI_STATE_INVENTORY:
      for (int i = 0; i < PLAYER_INVENTORY_SIZE; i++) {
        inventory_item_t item = game.player.inventory[i];
    
        uint8_t x = 3 + (i % 8) * 10;
        uint8_t y = 7 + (i / 8) * 15;

        if (item.id != ITEM_NONE) {
          render_count(item.count, x+1, y+9);
        }

        if (!ui.inventory_swapping || (i != ui.inventory_selected && i != ui.inventory_cursor) || (ui.frame_counter & 0b10)) {
          if (item.id != ITEM_NONE) {
            render_item(item.id, x, y);
          } else {
            lcd_draw_sprite(sprite_slot_empty[0], x, y);
          }
        }

        if (ui.menu_tab_selected && i == ui.inventory_cursor) {
          render_cursor(x, y);
        }
      }

      {
        machine_inventory_t machine_inventory;
        if (load_machine_inventory(&machine_inventory, game.player.x + ui.player_facing.x, game.player.y + ui.player_facing.y)) {
          // Show the machine's inventory.
          lcd_draw_sprite(sprite_recipe_colon[0], 8, 40);
          render_item(machine_inventory.item_id, 0, 40);

          for (int i = 0; i < machine_inventory.slot_count; i++) {
            uint8_t x = 12 + 18*i;
            uint8_t y = 39;

            inventory_item_t item = machine_inventory.items[i];

            if (item.id != ITEM_NONE) {
              render_count(item.count, x+9, y+3);
            }

            int cursor_i = i+PLAYER_INVENTORY_SIZE;
            if (!ui.inventory_swapping || (cursor_i != ui.inventory_selected && cursor_i != ui.inventory_cursor) || (ui.frame_counter & 0b10)) {
              if (item.id != ITEM_NONE) {
                render_item(item.id, x, y);
              } else {
                lcd_draw_sprite(sprite_slot_empty[0], x, y);
              }
            }

            if (ui.menu_tab_selected && cursor_i == ui.inventory_cursor) {
              render_cursor(x, y);
            }
          }
        }
      }

      break;

    case UI_STATE_CRAFT:
      {
        // Render selectable recipes.
        for (int i = 0; i <= recipe_count; i++) {
          uint8_t x = 3 + (i % 8) * 10;
          uint8_t y = 7 + (i / 8) * 9;
  
          if (i == recipe_count) {
            x = 73;
            lcd_draw_sprite(sprite_x[0], x, y);
          } else {
            render_item(recipes[i].result, x, y);
          }
          if (ui.menu_tab_selected && i == ui.recipe_cursor) {
            render_cursor(x, y);
          }
        }
  
        int16_t facing_y = game.player.y + ui.player_facing.y;
        int16_t facing_x = game.player.x + ui.player_facing.x;
        uint8_t facing_id = game.level[facing_y][facing_x].id;
  
        if (facing_id >= ITEM_ASSEMBLER && facing_id <= ITEM_ASSEMBLER3) {
          // Render assembler recipe
          render_item(ITEM_ASSEMBLER, 0, 28);
          lcd_draw_sprite(sprite_recipe_colon[0], 9, 28);
          int8_t diff = facing_id - ITEM_ASSEMBLER;
          int8_t dx = diff & 1;
          int8_t dy = diff >> 1;
          int16_t assembler_x = facing_x - dx;
          int16_t assembler_y = facing_y - dy;
          cell_t data_cell = game.level[assembler_y+1][assembler_x+1];
          uint8_t assembler_recipe = (data_cell.data & 0b11111) - 1;
          if (assembler_recipe != 0xff) {
            render_item(recipes[assembler_recipe].result, 3, 38);
            render_count(recipes[assembler_recipe].yield, 12, 40);
            lcd_draw_sprite(sprite_recipe_colon[0], 20, 38);
    
            for (int i = 0; i < 4; i++) {
              inventory_item_t item = recipes[assembler_recipe].items[i];
              if (item.id == ITEM_NONE) break;
    
              render_item(item.id, 27 + 19*i, 38);
              render_count(item.count, 36 + 19*i, 40);
            }
          }
        } else {
          // Render selected recipe.
          if (ui.menu_tab_selected && ui.recipe_cursor != recipe_count) {
            render_item(recipes[ui.recipe_cursor].result, 3, 26);
            render_count(recipes[ui.recipe_cursor].yield, 12, 28);
            lcd_draw_sprite(sprite_recipe_colon[0], 20, 26);
    
            for (int i = 0; i < 4; i++) {
              inventory_item_t item = recipes[ui.recipe_cursor].items[i];
              if (item.id == ITEM_NONE) break;
    
              render_item(item.id, 27 + 19*i, 26);
              render_count(item.count, 36 + 19*i, 28);
            }
          }
    
          // Render craft progress.
          {
            lcd_draw_bg(bg_cell_craft_progress[9], -6, 35);
            lcd_draw_bg(bg_cell_craft_progress[9], 82, 35);
            int progress_pixels = 0;
            if (ui.recipe_queue_count) {
              progress_pixels = 80 * ui.recipe_progress / (recipes[ui.recipe_queue[0]].time - 1);
            }
            for (int i = 0; i < 10; i++) {
              int cell_progress = progress_pixels - 8*i;
              if (cell_progress < 0) cell_progress = 0;
              if (cell_progress > 8) cell_progress = 8;
              lcd_draw_bg(bg_cell_craft_progress[cell_progress], 2+8*i, 35);
            }
          }
        }
  
        // Render recipe queue.
        for (int i = 0; i < 8; i++) {
          if (i == ui.recipe_queue_count) break;
  
          uint8_t item = recipes[ui.recipe_queue[i]].result;
          render_item(item, 10*i, 40);
        }
  
        if (ui.recipe_queue_count > 8) {
          lcd_draw_bg(bg_cell_dots[0], 79, 45);
        }
      }
      break;

    case UI_STATE_SETTINGS:
      {
        const int8_t save_x = 5;
        const int8_t delete_x = 31;
        const int8_t auto_x = 57;
        const int8_t y = 15;

        int visible_cursor = ui.menu_tab_selected ? ui.settings_cursor : -1;
        if (visible_cursor != 0 || (ui.frame_counter & 0b10)) {
          lcd_draw_bg(bg_cell_save[0], save_x, y);
          lcd_draw_bg(bg_cell_save[1], save_x+8, y);
          lcd_draw_bg(bg_cell_save[2], save_x, y+7);
          lcd_draw_bg(bg_cell_save[3], save_x+8, y+7);
        }
  
        if (visible_cursor != 1 || (ui.frame_counter & 0b10)) {
          lcd_draw_bg(bg_cell_delete[0], delete_x, y);
          lcd_draw_bg(bg_cell_delete[1], delete_x+8, y);
          lcd_draw_bg(bg_cell_delete[2], delete_x, y+7);
          lcd_draw_bg(bg_cell_delete[3], delete_x+8, y+7);
        }
  
        if (visible_cursor != 2 || (ui.frame_counter & 0b10)) {
          lcd_draw_bg(bg_cell_auto[0], auto_x, y);
          lcd_draw_bg(bg_cell_auto[1], auto_x+8, y);
          lcd_draw_bg(bg_cell_auto[game.autosave ? 4 : 2], auto_x, y+7);
          lcd_draw_bg(bg_cell_auto[game.autosave ? 5 : 3], auto_x+8, y+7);
        }

        // lcd contrast
        int8_t contrast_y = 38;
        uint8_t contrast = game.contrast >> 1;
        for (int i = 0; i < 64; i += 8) {
          lcd_draw_bg(bg_cell_line[0], 8+i, contrast_y);
        }
        
        if (visible_cursor != 3 || (ui.frame_counter & 0b10)) {
          lcd_draw_sprite(sprite_line_cursor[0], 8 + contrast, contrast_y);
        }
      }
      break;

    case UI_STATE_INFO:
    {
      uint32_t seconds = game.tick_counter % 60;
      uint32_t minutes_total = game.tick_counter / 60;
      uint32_t minutes = minutes_total % 60;
      uint32_t hours = minutes_total / 60;

      if (hours > 99) {
        hours = 99;
        minutes = 59;
        seconds = 59;
      }

      const int8_t time_y = 16;
      lcd_draw_bg(bg_cell_time[(ui.frame_counter >> 1) & 7], 26, time_y);

      lcd_draw_bg(bg_cell_time_colon[0], 44, time_y+1);
      lcd_draw_bg(bg_cell_time_colon[0], 54, time_y+1);

      render_2_digits(hours, 36, time_y+1);
      render_2_digits(minutes, 46, time_y+1);
      render_2_digits(seconds, 56, time_y+1);

      const int8_t science_y = 28;
      lcd_draw_sprite(sprite_science[0], 26, science_y);

      uint32_t science = game.science_counter;
      if (science > 9999999LL) {
        science = 9999999LL;
      }
      for (int i = 0; i < 7; i++) {
        lcd_draw_digit(science % 10, 60 - 4*i, science_y+2);
        science /= 10;
      }
    }

    default:
      break;
  }
}


void render_level() {
  int16_t player_pixel_x = 8*game.player.x + (2 * ui.player_subcell.x);
  int16_t player_pixel_y = 7*game.player.y + (7 * ui.player_subcell.y / 4);

  int16_t camera_x = player_pixel_x - (LCD_WIDTH/2-4);
  int16_t camera_y = player_pixel_y - (LCD_HEIGHT/2-3);

  if (camera_x < 0) camera_x = 0;
  if (camera_x > CAMERA_MAX_X) camera_x = CAMERA_MAX_X;
  if (camera_y < 0) camera_y = 0;
  if (camera_y > CAMERA_MAX_Y) camera_y = CAMERA_MAX_Y;

  uint16_t cell_left = camera_x / 8;
  uint16_t cell_right = (camera_x + LCD_WIDTH - 1) / 8;
  uint16_t cell_top = camera_y / 7;
  uint16_t cell_bottom = (camera_y + LCD_HEIGHT - 1) / 7;
  for (uint16_t cell_x = cell_left; cell_x <= cell_right; cell_x++) {
    for (uint16_t cell_y = cell_top; cell_y <= cell_bottom; cell_y++) {
      int8_t x = (cell_x * 8) - camera_x;
      int8_t y = (cell_y * 7) - camera_y;

      uint8_t id = game.level[cell_y][cell_x].id;
      if (ui.placing_item != ITEM_NONE && (ui.frame_counter & 0b10)) {
        if (cell_x >= ui.placing_x && cell_x < ui.placing_x + 2 && cell_y >= ui.placing_y && cell_y < ui.placing_y + 2) {
          int offset = (cell_x - ui.placing_x) + 2*(cell_y - ui.placing_y);
          if (ui.placing_blocked & (1 << offset)) {
            id = ITEM_ID_COUNT; // Use invalid ID.
          } else {
            id = ui.placing_item + offset;
          }
        }
      }

      const uint8_t *bg;
      switch(id) {
        case ITEM_NONE:
          bg = bg_cell_grass[((cell_y & 3) << 2) | (cell_x & 3)];
          break;
        case ITEM_ROCK:
          bg = bg_cell_rock[((cell_y & 3) << 2) | (cell_x & 3)];
          break;
        case ITEM_COAL:
          bg = bg_cell_coal[((cell_y & 3) << 2) | (cell_x & 3)];
          break;
        case ITEM_IRON:
          bg = bg_cell_iron_ore[((cell_y & 3) << 2) | (cell_x & 3)];
          break;
        case ITEM_COPPER:
          bg = bg_cell_copper_ore[((cell_y & 3) << 2) | (cell_x & 3)];
          break;

        case ITEM_WALL:
          bg = bg_cell_wall[0];
          break;

        case ITEM_DRILL:
          bg = bg_cell_drill[0];
          break;
        case ITEM_DRILL1:
          bg = bg_cell_drill[1];
          break;
        case ITEM_DRILL2:
          bg = bg_cell_drill[2];
          break;
        case ITEM_DRILL3:
          bg = bg_cell_drill[3];
          break;

        case ITEM_ASSEMBLER:
          bg = bg_cell_assembler[0];
          break;
        case ITEM_ASSEMBLER1:
          bg = bg_cell_assembler[1];
          break;
        case ITEM_ASSEMBLER2:
          bg = bg_cell_assembler[2];
          break;
        case ITEM_ASSEMBLER3:
          bg = bg_cell_assembler[3];
          break;

        case ITEM_FURNACE:
          bg = bg_cell_furnace[0];
          break;
        case ITEM_FURNACE1:
          bg = bg_cell_furnace[1];
          break;
        case ITEM_FURNACE2:
          bg = bg_cell_furnace[2];
          break;
        case ITEM_FURNACE3:
          bg = bg_cell_furnace[3];
          break;

        case ITEM_LAB:
          bg = bg_cell_lab[0];
          break;
        case ITEM_LAB1:
          bg = bg_cell_lab[1];
          break;
        case ITEM_LAB2:
          bg = bg_cell_lab[2];
          break;
        case ITEM_LAB3:
          bg = bg_cell_lab[3];
          break;
          
        case ITEM_ID_COUNT:
          bg = bg_cell_x[0];
          break;
        default:
          bg = bg_cell_blank[0];
          break;
      }

      lcd_draw_bg(bg, x, y);
    }
  }

  size_t sprite_index = ((ui.player_animation_timer >> 1) & 3);
  if (ui.player_facing.x > 0) {
    sprite_index += 24;
  } else if (ui.player_facing.x < 0) {
    sprite_index += 16;
  } else if (ui.player_facing.y > 0) {
    sprite_index += 8;
  }
  lcd_draw_sprite(sprite_player[sprite_index], player_pixel_x - camera_x, player_pixel_y - camera_y - 10);
  lcd_draw_sprite(sprite_player[sprite_index+4], player_pixel_x - camera_x, player_pixel_y - camera_y - 2);

  if (ui.action_progress) {
    lcd_draw_sprite(sprite_progress[ui.action_progress-1], player_pixel_x + 8*ui.player_facing.x - camera_x, player_pixel_y + 8*ui.player_facing.y - camera_y);
  }
}

void render() {
  if (ui.save_requested) {
    lcd_clear();
    int8_t save_x = 34;
    int8_t save_y = 16;
    lcd_draw_bg(bg_cell_save[0], save_x, save_y);
    lcd_draw_bg(bg_cell_save[1], save_x+8, save_y);
    lcd_draw_bg(bg_cell_save[2], save_x, save_y+7);
    lcd_draw_bg(bg_cell_save[3], save_x+8, save_y+7);
    lcd_refresh();
    return;
  }
  
  if (ui.state == UI_STATE_LEVEL) {
    render_level();
  } else {
    render_menu();
  }
  
  lcd_refresh();
}
