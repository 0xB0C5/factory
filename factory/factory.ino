#include "lcd.h"
#include "input.h"
#include "game.h"
#include "LittleFS.h"
#include "battery.h"
#include "ui.h"
#include "render.h"
#include "fatal_error.h"

// Uncomment for delay() to reduce power.
// #include <Snooze.h>

uint32_t frame_time;

void setup(void)
{
  // Serial.begin(9600);

  lcd_init();
  battery_init();

  input_init();

  level_init();
  if (!level_load()) {
    level_generate();
  }
  ui_reset();

  // Show the logo.
  delay(4000);

  frame_time = millis();
}

void loop(void)
{
  level_update(ui.level_subtick);
  ui_update();

  if (ui.delete_requested) {
    level_generate();
    ui_reset();
    return;
  }

  render();
  if (ui.save_requested && ui.level_subtick == 0) {
    ui.save_requested = false;
    if (!level_save()) {
      showError("SAVE FAILED.");
    }
  }

  uint32_t wait_duration = frame_time + 100 - millis();
  if (wait_duration > 100) {
    // Slowdown.
    frame_time = millis();
  } else {
    delay(wait_duration);
    frame_time += 100;
  }
}
