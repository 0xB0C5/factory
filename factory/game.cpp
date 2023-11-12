#include "game.h"
#include "Arduino.h"
#include "LittleFS.h"
#include "fatal_error.h"

#include <math.h>

game_t game;

const char *LEVEL_FILENAME = "level.bin";

LittleFS_Program fileSystem;

void level_init()
{
  if (!fileSystem.begin(1024L * 1024L)) {
    fatalError("Failed to start filesystem!");
  }
}

bool level_load() {
  File file = fileSystem.open(LEVEL_FILENAME, FILE_READ);

  if (!file) return false;

  size_t read_count = file.read(&game, sizeof(game));

  file.flush();
  file.close();

  return (read_count == sizeof(game));
}

bool level_save() {
  // TODO : ideally we'd do something that doesn't get borked if the power is cut.
  File file = fileSystem.open(LEVEL_FILENAME, FILE_WRITE);

  if (!file) return false;

  size_t written = file.write(&game, sizeof(game));

  file.flush();
  file.close();

  return (written == sizeof(game));
}

static const int resource_patch_counts[] = {999,        999,        999,        999};
static const int resource_patch_ids[]    = {ITEM_COAL, ITEM_ROCK, ITEM_IRON, ITEM_COPPER};

void level_generate(uint32_t seed) {
  memset(&game, 0, sizeof(game));

  randomSeed(seed);

  for (uint16_t i = 0; i < sizeof(resource_patch_counts)/sizeof(resource_patch_counts[0]); i++) {
    Serial.print("Generating resource #");
    Serial.println(i);

    for (int patch = 0; patch < resource_patch_counts[i]; patch++) {
      int size = random(8, 16);
      int y0 = random(LEVEL_HEIGHT_CELLS + 1 - size);
      int x0 = random(LEVEL_WIDTH_CELLS + 1 - size);

      for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
          float dist_x = dx - (size-1)/2.0f;
          float dist_y = dy - (size-1)/2.0f;
          float dist = sqrtf(dist_x*dist_x + dist_y*dist_y);
          if (dist*2 > size) continue;
          float density = (size - dist) / (float)size;
          uint16_t count = (uint16_t)(density * 1023);

          cell_t *cell = &(game.level[y0+dy][x0+dx]);
          if (count > cell->data) {
            cell->data = count;
            cell->id = resource_patch_ids[i];
          }
        }
      }
    }
  }

  game.player.x = LEVEL_WIDTH_CELLS/2;
  game.player.y = LEVEL_HEIGHT_CELLS/2;
}
