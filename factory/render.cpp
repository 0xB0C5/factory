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
    case ITEM_STORAGE:
      sprite = sprite_storage_icon[0];
      break;
    case ITEM_SWITCHER:
      sprite = sprite_switcher_icon[0];
      break;
    case ITEM_WIRE:
      sprite = sprite_wire[0];
      break;
    case ITEM_WOOD:
      sprite = sprite_wood[0];
      break;

    default:
      // Don't render.
      return;
  }

  lcd_draw_sprite(sprite, x, y);
}

void render_count(uint8_t count, int8_t x, int8_t y) {
  if (count >= 10) lcd_draw_digit(count / 10, x, y);
  lcd_draw_digit(count % 10, x + 4, y);
}

void render_cursor(int8_t x, int8_t y) {
  int cursor_index = ((ui.frame_counter >> 2) & 3) << 2;
  lcd_draw_sprite(sprite_cursor[cursor_index], x-1, y-1);
  lcd_draw_sprite(sprite_cursor[cursor_index|1], x+7, y-1);
  lcd_draw_sprite(sprite_cursor[cursor_index|2], x-1, y+7);
  lcd_draw_sprite(sprite_cursor[cursor_index|3], x+7, y+7);
}

void render_menu() {
  lcd_clear();

  uint8_t visible_state = ui.state;
  if (!ui.menu_tab_selected && (ui.frame_counter & 0b100)) {
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
        uint8_t y = 7 + (i / 8) * 16;

        if (item.id != ITEM_NONE) {
          render_item(item.id, x, y);
          render_count(item.count, x+1, y+9);
        }

        if (ui.menu_tab_selected && i == ui.inventory_cursor) {
          render_cursor(x, y);
        }
      }
      break;

    case UI_STATE_CRAFT:
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

      // Render selected recipe.
      if (ui.menu_tab_selected && ui.recipe_cursor != recipe_count) {
        render_item(recipes[ui.recipe_cursor].result, 0, 25);
        lcd_draw_sprite(sprite_recipe_colon[0], 9, 25);
        
        for (int i = 0; i < 4; i++) {
          inventory_item_t item = recipes[ui.recipe_cursor].items[i];
          if (item.id == ITEM_NONE) break;

          render_count(item.count, 14 + 18*i, 27);
          render_item(item.id, 22 + 18*i, 25);
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

      // Render recipe queue.
      for (int i = 0; i < 8; i++) {
        if (i == ui.recipe_queue_count) break;

        uint8_t item = recipes[ui.recipe_queue[i]].result;
        render_item(item, 10*i, 40);
      }

      break;

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
      if (ui.placing_item != ITEM_NONE && (ui.frame_counter & 0b100)) {
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
  if (ui.state == UI_STATE_LEVEL) {
    render_level();
  } else {
    render_menu();
  }
  
  lcd_refresh();
}
