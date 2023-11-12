#include "lcd.h"
#include "input.h"
#include "game.h"
#include "generated_graphics.h"
#include "LittleFS.h"
#include "battery.h"
// Uncomment for delay() to reduce power.
// #include <Snooze.h>

void setup(void)
{
  Serial.begin(9600);

  /*
  if (!fileSystem.begin(1024L * 1024L)) {
    while (true) {
      Serial.println("FS INIT FAILED!");
      delay(1000);
    }
  }

  uint32_t x = 0;
  File testFile = fileSystem.open("test.bin", FILE_READ);
  if (testFile) {
    Serial.println("GOT FILE!");
    size_t count = testFile.read(&x, sizeof(x));
    if (count != sizeof(x)) {
      Serial.println("WRONG SIZE?");
      x = 0;
    }
  } else {
    Serial.println("NO FILE!");
  }

  Serial.print("GOT: ");
  Serial.println(x);
  x++;
  Serial.print("SAVING: ");
  Serial.println(x);

  testFile.close();
  testFile = fileSystem.open("test.bin", FILE_WRITE);
  if (!testFile) {
    Serial.println("CAN'T WRITE!");
  } else {
    size_t count = testFile.write(&x, sizeof(x));
    if (count != sizeof(x)) {
      Serial.println("WRONG SIZE?");
    }
    testFile.flush();
  }
  testFile.close();
  Serial.println("DONE!");
  */

  lcd_init();
  lcd_refresh();
  battery_init();

  input_init();

  level_init();
  level_generate(1234);
}

int16_t player_x = 16;
int16_t player_y = 16;

int8_t player_dx = 0;
int8_t player_dy = 0;
int8_t player_facing_x = 0;
int8_t player_facing_y = 1;

uint8_t player_animation_index = 0;

uint8_t action_progress = 0;

#define PLAYER_SPEED 2
#define CAMERA_MAX_X (512-LCD_WIDTH)
#define CAMERA_MAX_Y (512-LCD_HEIGHT)

uint16_t frame_counter;

void loop(void)
{
  frame_counter++;
  uint8_t input = input_read();
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

  int16_t player_pixel_x = player_x;
  int16_t player_pixel_y = player_y * 7 / 8;

  int16_t camera_x = player_pixel_x - (LCD_WIDTH/2-4);
  int16_t camera_y = player_pixel_y - (LCD_HEIGHT/2-2);

  if (camera_x < 0) camera_x = 0;
  if (camera_x > CAMERA_MAX_X) camera_x = CAMERA_MAX_X;
  if (camera_y < 0) camera_y = 0;
  if (camera_y > CAMERA_MAX_Y) camera_y = CAMERA_MAX_Y;

  // lcd_clear();
  // TODO : these are CELLS not TILES.
  uint16_t tile_left = camera_x / 8;
  uint16_t tile_right = (camera_x + LCD_WIDTH - 1) / 8;
  uint16_t tile_top = camera_y / 7;
  uint16_t tile_bottom = (camera_y + LCD_HEIGHT - 1) / 7;
  for (uint16_t tile_x = tile_left; tile_x <= tile_right; tile_x++) {
    for (uint16_t tile_y = tile_top; tile_y <= tile_bottom; tile_y++) {
      int8_t x = (tile_x * 8) - camera_x;
      int8_t y = (tile_y * 7) - camera_y;

      cell_t cell = game.level[tile_y][tile_x];
      const uint8_t *bg;
      switch(cell.id) {
        case RESOURCE_NONE:
          bg = bg_tile_grass[((tile_y & 3) << 2) | (tile_x & 3)];
          break;
        case RESOURCE_ROCK:
          bg = bg_tile_rock[((tile_y & 3) << 2) | (tile_x & 3)];
          break;
        case RESOURCE_COAL:
          bg = bg_tile_coal[((tile_y & 3) << 2) | (tile_x & 3)];
          break;
        case RESOURCE_IRON:
          bg = bg_tile_iron_ore[((tile_y & 3) << 2) | (tile_x & 3)];
          break;
        case RESOURCE_COPPER:
          bg = bg_tile_copper_ore[((tile_y & 3) << 2) | (tile_x & 3)];
          break;
        default:
          bg = bg_tile_blank[0];
          break;
      }
      
      lcd_draw_bg(bg, x, y);
    }
  }

  size_t sprite_index = ((player_animation_index >> 1) & 3);
  if (player_facing_x > 0) {
    sprite_index += 24;
  } else if (player_facing_x < 0) {
    sprite_index += 16;
  } else if (player_facing_y > 0) {
    sprite_index += 8;
  }
  lcd_draw_sprite(sprite_player[sprite_index], player_pixel_x - camera_x, player_pixel_y - camera_y - 10);
  lcd_draw_sprite(sprite_player[sprite_index+4], player_pixel_x - camera_x, player_pixel_y - camera_y - 2);

  if (action_progress) {
    lcd_draw_sprite(sprite_progress[action_progress-1], player_pixel_x + 8*player_facing_x - camera_x, player_pixel_y + 8*player_facing_y - camera_y);
  }

  lcd_refresh();

  delay(100);
}
