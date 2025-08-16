#include <stdint.h>

#define INPUT_UP 1
#define INPUT_DOWN 2
#define INPUT_LEFT 4
#define INPUT_RIGHT 8
#define INPUT_A 16
#define INPUT_B 32

#define PIN_LEFT 17
#define PIN_UP 18
#define PIN_DOWN 19
#define PIN_RIGHT 20
#define PIN_B 21
#define PIN_A 22

void input_init();
uint8_t input_read();
