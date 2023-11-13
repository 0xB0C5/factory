#include "input.h"
#include "Arduino.h"

#define PIN_LEFT 17
#define PIN_UP 18
#define PIN_DOWN 19
#define PIN_RIGHT 20
#define PIN_B 21
#define PIN_A 22

const int16_t INPUT_PINS[] = {  PIN_A,   PIN_B,   PIN_RIGHT,   PIN_DOWN,   PIN_UP,   PIN_LEFT};
const uint8_t INPUTS[]     = {INPUT_A, INPUT_B, INPUT_RIGHT, INPUT_DOWN, INPUT_UP, INPUT_LEFT};

#define INPUT_COUNT 6

void input_init() {
  for (int i = 0; i < INPUT_COUNT; i++) {
    pinMode(INPUT_PINS[i], INPUT_PULLUP);
  }
}

uint8_t input_read() {
  uint8_t res = 0;
  for (int i = 0; i < INPUT_COUNT; i++) {
    if (digitalRead(INPUT_PINS[i]) == LOW) {
      res |= INPUTS[i];
    }
  }

	return res;
}
