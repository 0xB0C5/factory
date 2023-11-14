#include "lcd.h"
#include "input.h"

void fatalError(const char *message) {
	lcd_clear();
	lcd_string("FATAL ERROR:", 0, 0);
	lcd_string(message, 1, 0);
	lcd_refresh();
	
	while (true);
}

void showError(const char *message) {
  lcd_clear();
  lcd_string("ERROR:", 0, 0);
  lcd_string(message, 1, 0);
  lcd_string("PRESS A", 3, 0);
  lcd_refresh();

  delay(1000);
  while (!(input_read() & INPUT_A));
}
