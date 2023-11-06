#include "Arduino.h"

#define LCD_WIDTH  84
#define LCD_HEIGHT 48

void lcd_init();

void lcd_refresh();

void lcd_clear();

void lcd_draw_sprite(const byte *sprite, int8_t x0, int8_t y0);
void lcd_draw_bg(const byte *bg, int8_t x0, int8_t y0);
