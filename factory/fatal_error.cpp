#include "lcd.h"

void fatalError(const char *message) {
	lcd_clear();
	lcd_string("FATAL ERROR:", 0, 0);
	lcd_string(message, 1, 0);
	lcd_refresh();
	
	while (true);
}
