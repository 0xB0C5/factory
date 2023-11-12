#include "ui.h"
#include <string.h>

ui_t ui;

void ui_reset() {
  memset(&ui, 0, sizeof(ui));
}
