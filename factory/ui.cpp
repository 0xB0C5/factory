#include "ui.h"
#include <string.h>

ui_t ui;

void ui_reset() {
  memset(&ui, 0, sizeof(ui));

  ui.player_facing.y = 1;
  ui.inventory_selected = 0xff;
}
