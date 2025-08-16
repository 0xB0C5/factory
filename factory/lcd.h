#include <cstdint>

#define LCD_WIDTH  84
#define LCD_HEIGHT 48

extern uint8_t screen[LCD_WIDTH * LCD_HEIGHT / 8];

void lcd_init();

void lcd_refresh();

void lcd_clear();

void lcd_draw_sprite(const uint8_t *sprite, int8_t x0, int8_t y0);
void lcd_draw_bg(const uint8_t *bg, int8_t x0, int8_t y0);
void lcd_draw_digit(int8_t d, int8_t x0, int8_t y0);
void lcd_string(const char *message, int8_t row, int8_t x);
void lcd_set_contrast(uint8_t brightness);
uint8_t lcd_get_contrast();
