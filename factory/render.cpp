#include "render.h"
#include "ui.h"
#include "lcd.h"
#include "game.h"
#include "generated_graphics.h"

#define CAMERA_MAX_X (8*LEVEL_WIDTH_CELLS-LCD_WIDTH)
#define CAMERA_MAX_Y (7*LEVEL_HEIGHT_CELLS-LCD_HEIGHT)

void render() {
  int16_t player_pixel_x = 8*game.player.x + (2 * ui.player_subcell.x);
  int16_t player_pixel_y = 7*game.player.y + (7 * ui.player_subcell.y / 4);

  int16_t camera_x = player_pixel_x - (LCD_WIDTH/2-4);
  int16_t camera_y = player_pixel_y - (LCD_HEIGHT/2-2);

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

      cell_t cell = game.level[cell_y][cell_x];
      const uint8_t *bg;
      switch(cell.id) {
        case RESOURCE_NONE:
          bg = bg_cell_grass[((cell_y & 3) << 2) | (cell_x & 3)];
          break;
        case RESOURCE_ROCK:
          bg = bg_cell_rock[((cell_y & 3) << 2) | (cell_x & 3)];
          break;
        case RESOURCE_COAL:
          bg = bg_cell_coal[((cell_y & 3) << 2) | (cell_x & 3)];
          break;
        case RESOURCE_IRON:
          bg = bg_cell_iron_ore[((cell_y & 3) << 2) | (cell_x & 3)];
          break;
        case RESOURCE_COPPER:
          bg = bg_cell_copper_ore[((cell_y & 3) << 2) | (cell_x & 3)];
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

  lcd_refresh();
}
