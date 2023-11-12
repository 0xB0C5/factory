#include "lcd.h"
#include "input.h"
#include "game.h"
#include "LittleFS.h"
#include "battery.h"
#include "ui.h"
#include "render.h"
// Uncomment for delay() to reduce power.
// #include <Snooze.h>

uint32_t frame_time;

void setup(void)
{
  Serial.begin(9600);

  lcd_init();
  battery_init();

  input_init();

  level_init();
  level_generate(1234);

  // TODO : REMOVE?
  // This shows the logo.
  delay(4000);

  frame_time = millis();
}

void loop(void)
{
  ui_update();
  render();

  uint32_t wait_duration = frame_time + 100 - millis();
  if (wait_duration > 100) {
    // Slowdown.
    frame_time = millis();
  } else {
    delay(wait_duration);
  }
}
