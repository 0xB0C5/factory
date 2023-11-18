#include "game.h"
#include "Arduino.h"
#include "LittleFS.h"
#include "fatal_error.h"
#include "recipe.h"

#include <math.h>

game_t game;

const char *LEVEL_FILENAME = "level.bin";
const char *TEMP_FILENAME = "temp.bin";

LittleFS_Program fileSystem;

void printDirectory(File dir, int numSpaces) {
   while(true) {
     File entry = dir.openNextFile();
     if (!entry) {
       break;
     }
     // printSpaces(numSpaces);
     Serial.println(entry.name());
     /*
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       printSpaces(36 - numSpaces - strlen(entry.name()));
       Serial.print("  ");
       Serial.println(entry.size(), DEC);
     }
     */
     entry.close();
   }
}

void level_init()
{
  if (!fileSystem.begin(1024L * 1024L)) {
    fatalError("NO FILESYSTEM");
  }

  // Serial.println("FILES:");
  // printDirectory(fileSystem.open("/"), 0);
}


bool level_load() {
  File file = fileSystem.open(LEVEL_FILENAME, FILE_READ);

  if (!file) file = fileSystem.open(TEMP_FILENAME, FILE_READ);

  if (!file) return false;

  size_t read_count = file.read(&game, sizeof(game));

  file.flush();
  file.close();

  return (read_count == sizeof(game));
}

bool level_save() {
  // Remove temp file.
  fileSystem.remove(TEMP_FILENAME);

  game.save_tick = game.tick_counter;

  // Write to a temp file in case power is cut.
  File file = fileSystem.open(TEMP_FILENAME, FILE_WRITE);

  if (!file) {
    return false;
  }

  size_t written = file.write(&game, sizeof(game));

  file.flush();
  file.close();

  if (written != sizeof(game)) {
    return false;
  }

  // Now that we've written the temp file, we can safely delete the main file.
  fileSystem.remove(LEVEL_FILENAME);
  fileSystem.rename(TEMP_FILENAME, LEVEL_FILENAME);

  // Even if the rename didn't work, the temp file still exists, so this can be considered a success.
  return true;
}

static const int resource_patch_counts[] = {150,       150,       150,       150};
static const int resource_patch_ids[]    = {ITEM_COAL, ITEM_ROCK, ITEM_IRON, ITEM_COPPER};

void level_generate() {
  // Preserve contrast, autosave.
  uint8_t contrast = game.contrast;
  bool autosave = game.autosave;

  memset(&game, 0, sizeof(game));
  game.contrast = contrast;
  game.autosave = autosave;

  uint32_t seed = ARM_DWT_CYCCNT;
  randomSeed(seed);

  for (uint16_t i = 0; i < sizeof(resource_patch_counts)/sizeof(resource_patch_counts[0]); i++) {
    for (int patch = 0; patch < resource_patch_counts[i]; patch++) {
      int size = random(7, 12);
      int y0 = random(LEVEL_HEIGHT_CELLS + 1 - size);
      int x0 = random(LEVEL_WIDTH_CELLS + 1 - size);

      for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
          float dist_x = dx - (size-1)/2.0f;
          float dist_y = dy - (size-1)/2.0f;
          float dist = sqrtf(dist_x*dist_x + dist_y*dist_y);
          if (dist*2 > size) continue;
          float density = (size - dist) / (float)size;
          uint16_t count = (uint16_t)(density * 511);

          cell_t *cell = &(game.level[y0+dy][x0+dx]);
          if (count > cell->data) {
            cell->data = count;
            cell->id = resource_patch_ids[i];
          }
        }
      }
    }
  }

  // Delete adjacent resources so drills never have multiple resources.
  cell_t empty = {ITEM_NONE, 0};
  for (int y = 0; y < LEVEL_HEIGHT_CELLS-1; y++) {
    for (int x = 0; x < LEVEL_WIDTH_CELLS-1; x++) {
      if (game.level[y][x].id == ITEM_NONE) continue;

      if (game.level[y][x+1].id != game.level[y][x].id) {
        game.level[y][x+1] = empty;
      }
      if (game.level[y+1][x].id != game.level[y][x].id) {
        game.level[y+1][x] = empty;
      }
      if (game.level[y+1][x+1].id != game.level[y][x].id) {
        game.level[y+1][x+1] = empty;
      }
    }
  }

  // Block off edges of the map.
  cell_t blocked = {ITEM_WALL, 0};
  for (int y = 0; y < LEVEL_HEIGHT_CELLS; y++) {
    for (int x = 0; x < 4; x++) {
      game.level[y][x] = blocked;
    }
    for (int x = LEVEL_WIDTH_CELLS-4; x < LEVEL_WIDTH_CELLS; x++) {
      game.level[y][x] = blocked;
    }
  }
  for (int x = 0; x < LEVEL_WIDTH_CELLS; x++) {
    for (int y = 0; y < 4; y++) {
      game.level[y][x] = blocked;
    }
    for (int y = LEVEL_HEIGHT_CELLS-4; y < LEVEL_HEIGHT_CELLS; y++) {
      game.level[y][x] = blocked;
    }
  }

  game.player.x = LEVEL_WIDTH_CELLS/2;
  game.player.y = LEVEL_HEIGHT_CELLS/2;
}

bool load_machine_inventory(machine_inventory_t *inventory, int16_t x, int16_t y) {
  cell_t cell = game.level[y][x];
  // If not a machine, it has no inventory.
  if (cell.id < ITEM_MACHINES_START) return false;

  uint8_t offset = cell.id & 3;
  uint8_t base_id = cell.id - offset;
  int x0 = x - (offset & 1);
  int y0 = y - (offset >> 1);

  switch (base_id) {
    case ITEM_FURNACE:
    case ITEM_ASSEMBLER:
      inventory->slot_count = 3;
      inventory->slot_capacity = 64;
      break;
    case ITEM_DRILL:
      inventory->slot_count = 2;
      inventory->slot_capacity = 64;
      break;
    case ITEM_LAB:
      inventory->slot_count = 1;
      inventory->slot_capacity = 64;
      break;
    default:
      return false;
  }

  inventory->item_id = base_id;

  for (int i = 0; i < inventory->slot_count; i++) {
    uint16_t d = game.level[y0 + (i>>1)][x0 + (i&1)].data;
    inventory_item_t item = {(uint8_t)(d & 0b11111), (uint8_t)(d >> 5)};
    if (item.id != ITEM_NONE && item.count == 0) {
      item.count = 64;
    }
    inventory->items[i] = item;
  }

  return true;
}

bool store_machine_inventory(machine_inventory_t *inventory, int16_t x, int16_t y) {
  cell_t cell = game.level[y][x];
  // If not a machine, it has no inventory.
  if (cell.id < ITEM_MACHINES_START) return false;

  uint8_t offset = cell.id & 3;
  uint8_t base_id = cell.id - offset;
  int x0 = x - (offset & 1);
  int y0 = y - (offset >> 1);

  if (inventory->item_id != base_id) return false;

  switch (base_id) {
    case ITEM_FURNACE:
    case ITEM_ASSEMBLER:
      if (inventory->slot_count != 3) return false;
      if (inventory->slot_capacity != 64) return false;
      break;
    case ITEM_DRILL:
      if (inventory->slot_count != 2) return false;
      if (inventory->slot_capacity != 64) return false;
      break;
    case ITEM_LAB:
      if (inventory->slot_count != 1) return false;
      if (inventory->slot_capacity != 64) return false;
      break;
    default:
      return false;
  }

  for (int i = 0; i < inventory->slot_count; i++) {
    inventory_item_t item = inventory->items[i];
    uint16_t d = item.id | ((item.count & 0b111111) << 5);
    game.level[y0 + (i>>1)][x0 + (i&1)].data = d;
  }

  return true;
}


bool machine_craft_recipe(recipe_t *recipe, machine_inventory_t *inventory) {
  // Check we have enough items.
  for (int i = 0; i < 4; i++) {
    inventory_item_t recipe_item = recipe->items[i];
    if (recipe_item.id == ITEM_NONE) break;
    int count = 0;
    for (int j = 0; j < inventory->slot_count; j++) {
      inventory_item_t inv_item = inventory->items[j];
      if (inv_item.id == recipe_item.id) {
        count += inv_item.count;
        if (count >= recipe_item.count) break;
      }
    }
    if (count < recipe_item.count) return false;
  }

  // Find a slot we can put the items in.
  int slot = -1;
  for (int i = 0; i < inventory->slot_count; i++) {
    inventory_item_t inv_item = inventory->items[i];
    if (inv_item.id == ITEM_NONE || (inv_item.id == recipe->result && inv_item.count + recipe->yield <= inventory->slot_capacity)) {
      slot = i;
      break;
    }
  }

  if (slot == -1) return false;

  // Remove the items.
  for (int i = 0; i < 4; i++) {
    inventory_item_t recipe_item = recipe->items[i];
    if (recipe_item.id == ITEM_NONE) break;
    int remaining = recipe_item.count;
    for (int j = 0; j < inventory->slot_count; j++) {
      inventory_item_t *inv_item = &inventory->items[j];
      if (inv_item->id == recipe_item.id) {
        if (inv_item->count > remaining) {
          inv_item->count -= remaining;
          remaining = 0;
        } else {
          remaining -= inv_item->count;
          inv_item->count = 0;
          inv_item->id = ITEM_NONE;
        }
        if (remaining == 0) break;
      }
    }
  }

  // Add the item.
  inventory_item_t *new_item = &inventory->items[slot];
  new_item->id = recipe->result;
  new_item->count += recipe->yield;

  return true;
}

void update_furnace(int x, int y) {
  machine_inventory_t inventory;
  if (!load_machine_inventory(&inventory, x, y)) return;

  uint16_t burn_remainder = game.level[y+1][x+1].data;
  if (burn_remainder) {
    burn_remainder--;
    game.level[y+1][x+1].data = burn_remainder;
  }

  int coal_index = -1;
  int iron_index = -1;
  int copper_index = -1;
  int iron_plate_index = -1;
  int copper_plate_index = -1;
  for (int i = inventory.slot_count-1; i >= 0; i--) {
    inventory_item_t item = inventory.items[i];
    if (item.id == ITEM_NONE) {
      iron_plate_index = i;
      copper_plate_index = i;
    } else if (item.id == ITEM_COAL) {
      coal_index = i;
    } else if (item.id == ITEM_IRON) {
      iron_index = i;
    } else if (item.id == ITEM_COPPER) {
      copper_index = i;
    } else if (item.id == ITEM_IRON_PLATE) {
      if (item.count < 64) iron_plate_index = i;
    } else if (item.id == ITEM_COPPER_PLATE) {
      if (item.count < 64) copper_plate_index = i;
    }
  }

  if (burn_remainder == 0 && coal_index == -1) {
    // No fuel.
    return;
  }

  int input_index = -1;
  int output_index = -1;
  uint8_t output_id;
  if (iron_index != -1 && iron_plate_index != -1) {
    input_index = iron_index;
    output_index = iron_plate_index;
    output_id = ITEM_IRON_PLATE;
  } else if (copper_index != -1 && copper_plate_index != -1) {
    input_index = copper_index;
    output_index = copper_plate_index;
    output_id = ITEM_COPPER_PLATE;
  }

  if (input_index == -1) return;

  // Smelt!
  if (burn_remainder == 0) {
    inventory.items[coal_index].count--;
    if (inventory.items[coal_index].count == 0) inventory.items[coal_index].id = ITEM_NONE;
    game.level[y+1][x+1].data = 8;
  }

  inventory.items[input_index].count--;
  if (inventory.items[input_index].count == 0) inventory.items[input_index].id = ITEM_NONE;

  inventory.items[output_index].count++;
  inventory.items[output_index].id = output_id;

  store_machine_inventory(&inventory, x, y);
}

void update_drill(int x, int y) {
  machine_inventory_t inventory;
  if (!load_machine_inventory(&inventory, x, y)) return;

  uint16_t data2 = game.level[y+1][x].data;
  uint8_t resource_id = data2 & 0b11111;
  uint16_t burn_remainder = data2 >> 5;
  if (burn_remainder) {
    burn_remainder--;
    game.level[y+1][x].data = (burn_remainder << 5) | resource_id;
  }

  if (game.level[y+1][x+1].data == 0) return; // Nothing to mine!

  int coal_index = -1;
  int output_index = -1;
  for (int i = inventory.slot_count-1; i >= 0; i--) {
    inventory_item_t item = inventory.items[i];
    if (item.id == ITEM_NONE) {
      output_index = i;
    } else {
      if (item.id == ITEM_COAL) {
        coal_index = i;
      }
      if (item.id == resource_id) {
        if (item.count < 64) output_index = i;
      }
    }
  }

  if (burn_remainder == 0 && coal_index == -1) {
    // No fuel.
    return;
  }

  // Nowhere to place it.
  if (output_index == -1) return;

  // Mine!
  if (burn_remainder == 0) {
    inventory.items[coal_index].count--;
    if (inventory.items[coal_index].count == 0) inventory.items[coal_index].id = ITEM_NONE;
    // Update burn remainder.
    game.level[y+1][x].data = (8 << 5) | resource_id;
  }

  game.level[y+1][x+1].data--;

  inventory.items[output_index].count++;
  inventory.items[output_index].id = resource_id;

  store_machine_inventory(&inventory, x, y);
}

void update_lab(int x, int y) {
  machine_inventory_t inventory;
  if (!load_machine_inventory(&inventory, x, y)) return;

  for (int i = 0; i < inventory.slot_count; i++) {
    if (inventory.items[i].id == ITEM_SCIENCE) {
      inventory.items[i].count--;
      if (inventory.items[i].count == 0) {
        inventory.items[i].id = ITEM_NONE;
      }
      game.science_counter++;
      store_machine_inventory(&inventory, x, y);
      return;
    }
  }
}

void update_assembler(int x, int y) {
  machine_inventory_t inventory;
  if (!load_machine_inventory(&inventory, x, y)) return;

  uint16_t d3 = game.level[y+1][x+1].data;
  uint8_t recipe_index = (d3 & 0b11111) - 1;
  if (recipe_index == 0xff) {
    // Mark as not in progress.
    game.level[y+1][x+1].data = 0;
    return;
  }

  const recipe_t *recipe = &recipes[recipe_index];

  // Remove the items for the recipe.
  for (int i = 0; i < 4; i++) {
    uint8_t id = recipe->items[i].id;
    if (id == ITEM_NONE) break;

    int remain = recipe->items[i].count;
    for (int j = 0; j < inventory.slot_count; j++) {
      if (inventory.items[j].id == id) {
        int remove;
        if (inventory.items[j].count <= remain) {
          remove = inventory.items[j].count;
        } else {
          remove = remain;
        }

        remain -= remove;
        inventory.items[j].count -= remove;
        if (inventory.items[j].count == 0) inventory.items[j].id = ITEM_NONE;

        if (remain == 0) break;
      }
    }

    if (remain > 0) {
      // Can't do the recipe.
      // Mark as not in progress, with the existing recipe index.
      // (Don't save the inventory, so the change doesn't get persisted.)
      game.level[y+1][x+1].data = recipe_index + 1;
      return;
    }
  }

  // Find a slot for the output.
  int output_index = -1;
  for (int i = 0; i < inventory.slot_count; i++) {
    uint8_t id = inventory.items[i].id;
    if (id == ITEM_NONE || (id == recipe->result && inventory.items[i].count + recipe->yield <= inventory.slot_capacity)) {
      output_index = i;
      break;
    }
  }

  if (output_index == -1) {
    // Can't do the recipe.
    // Mark as not in progress, with the existing recipe index.
    // (Don't save the inventory, so the change doesn't get persisted.)
    game.level[y+1][x+1].data = recipe_index + 1;
    return;
  }

  // Can do the recipe!
  uint8_t progress = (((d3 >> 5) & 0b11111) + 1) << 4;
  if (progress >= recipe->time) {
    progress = 0;
    inventory.items[output_index].count += recipe->yield;
    inventory.items[output_index].id = recipe->result;
    store_machine_inventory(&inventory, x, y);
  }

  // In progress, progress, recipe index
  game.level[y+1][x+1].data = 0b10000000000 | (progress << 1) | (recipe_index + 1);
}

void level_update(uint8_t subtick) {
  if (subtick == LEVEL_SUBTICKS-1) {
    game.tick_counter++;
    return;
  }

  for (int y = subtick; y < LEVEL_HEIGHT_CELLS; y += LEVEL_SUBTICKS - 1) {
    for (int x = 0; x < LEVEL_WIDTH_CELLS; x++) {
      switch (game.level[y][x].id) {
        case ITEM_FURNACE:
          {
            // Furnaces update every 4 ticks.
            // Stagger ticks using x coordinate (since y coordinate determines subtick).
            uint8_t update_tick = (x >> 2) & 3;
            if ((game.tick_counter & 3) == update_tick) {
              update_furnace(x, y);
            }
            break;
          }
        case ITEM_LAB:
          {
            // Labs update every 8 ticks.
            uint8_t update_tick = (x >> 2) & 7;
            if ((game.tick_counter & 7) == update_tick) {
              update_lab(x, y);
            }
            break;
          }
        case ITEM_DRILL:
          {
            // Drills update every 4 ticks.
            uint8_t update_tick = (x >> 2) & 3;
            if ((game.tick_counter & 3) == update_tick) {
              update_drill(x, y);
            }
            break;
          }
        case ITEM_ASSEMBLER:
          {
            // Assemblers update every 4 ticks.
            uint8_t update_tick = (x >> 2) & 3;
            if ((game.tick_counter & 3) == update_tick) {
              update_assembler(x, y);
            }
            break;
          }
        default:
          break;
      }
    }
  }
}
