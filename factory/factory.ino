#include "lcd.h"
#include "input.h"
#include "level.h"
#include "generated_graphics.h"

void setup(void)
{
  lcd_init();

  lcd_refresh();
  randomSeed(1234);
  gen_level();
}

int16_t player_x = 4*LEVEL_WIDTH;
int16_t player_y = 4*LEVEL_HEIGHT;

int8_t player_dx = 0;
int8_t player_dy = 0;
int8_t player_facing_x = 0;
int8_t player_facing_y = 1;

uint8_t player_animation_index = 0;

uint8_t action_progress = 0;

#define PLAYER_SPEED 1
#define CAMERA_MAX_X (8*LEVEL_WIDTH-LCD_WIDTH)
#define CAMERA_MAX_Y (8*LEVEL_HEIGHT-LCD_HEIGHT)

void loop(void)
{
  uint8_t input = read_input();
  bool is_player_aligned = player_x % 8 == 0 && player_y % 8 == 0;
  if (is_player_aligned) {
    player_dx = 0;
    player_dy = 0;
    if (input & INPUT_UP) {
      player_dy -= 1;
    }

    if (input & INPUT_DOWN) {
      player_dy += 1;
    }

    if (input & INPUT_LEFT) {
      player_dx -= 1;
    }

    if (input & INPUT_RIGHT) {
      player_dx += 1;
    }
  }

  player_x += player_dx * PLAYER_SPEED;
  player_y += player_dy * PLAYER_SPEED;

  if (player_dx != 0 || player_dy != 0) {
    player_animation_index++;
    player_facing_x = player_dx;
    player_facing_y = player_dy;
  } else {
    player_animation_index = 0;
  }

  if (is_player_aligned && ((input & INPUT_A) != 0) && action_progress < 13) {
    action_progress++;
  } else {
    action_progress = 0;
  }

  int16_t camera_x = player_x - (LCD_WIDTH/2-4);
  int16_t camera_y = player_y - (LCD_HEIGHT/2-4);

  if (camera_x < 0) camera_x = 0;
  if (camera_x > CAMERA_MAX_X) camera_x = CAMERA_MAX_X;
  if (camera_y < 0) camera_y = 0;
  if (camera_y > CAMERA_MAX_Y) camera_y = CAMERA_MAX_Y;

  // lcd_clear();
  uint16_t tile_left = camera_x >> 3;
  uint16_t tile_right = (camera_x + LCD_WIDTH - 1) >> 3;
  uint16_t tile_top = camera_y >> 3;
  uint16_t tile_bottom = (camera_y + LCD_HEIGHT - 1) >> 3;
  for (uint16_t tile_x = tile_left; tile_x <= tile_right; tile_x++) {
    for (uint16_t tile_y = tile_top; tile_y <= tile_bottom; tile_y++) {
      int8_t x = (tile_x << 3) - camera_x;
      int8_t y = (tile_y << 3) - camera_y;

      tile_t tile = get_tile(tile_x, tile_y);

      lcd_draw_bg(tile ? bg_tile_rock[0] : bg_tile_blank[0], x, y);
    }
  }

  size_t sprite_index = ((player_animation_index >> 2) & 3);
  if (player_facing_x > 0) {
    sprite_index += 24;
  } else if (player_facing_x < 0) {
    sprite_index += 16;
  } else if (player_facing_y > 0) {
    sprite_index += 8;
  }
  lcd_draw_sprite(sprite_player[sprite_index], player_x - camera_x, player_y - camera_y - 8);
  lcd_draw_sprite(sprite_player[sprite_index+4], player_x - camera_x, player_y - camera_y);

  if (action_progress) {
    lcd_draw_sprite(sprite_progress[action_progress-1], player_x + 8*player_facing_x - camera_x, player_y + 8*player_facing_y - camera_y);
  }
  
  lcd_refresh();
  
  delay(50);
}
