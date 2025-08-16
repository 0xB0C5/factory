#include "lcd.h"
#include "input.h"
#include "arduino_compat.h"

#ifdef WEB
#include <cstdio>
#endif

void fatalError(const char *message) {
#ifdef WEB
  printf("FATAL ERROR: %s\n", message);
#else
	lcd_clear();
	lcd_string("FATAL ERROR:", 0, 0);
	lcd_string(message, 1, 0);
	lcd_refresh();
	
	while (true);
#endif
}

void showError(const char *message) {
#ifdef WEB
  printf("ERROR: %s\n", message);
#else
  lcd_clear();
  lcd_string("ERROR:", 0, 0);
  lcd_string(message, 1, 0);
  lcd_string("PRESS A", 3, 0);
  lcd_refresh();

  delay(1000);
  while (!(input_read() & INPUT_A));
#endif
}
