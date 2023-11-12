#include <stdint.h>

#define INPUT_UP 1
#define INPUT_DOWN 2
#define INPUT_LEFT 4
#define INPUT_RIGHT 8
#define INPUT_A 16
#define INPUT_B 32

void input_init();
uint8_t input_read();
