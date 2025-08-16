#ifdef WEB
#include <cstdio>
#include <memory>
#include "factory.h"
#include "arduino_compat.h"
#include "lcd.h"
#include "input.h"

uint32_t seed;

extern "C" {

void setup(uint32_t s) {
    seed = s;
    factory_setup();
}

void loop() {
    factory_loop();
}

uint8_t *getScreen() {
    return screen;
}

void setInputs(uint8_t inputs) {
    digitalWrite(PIN_LEFT, inputs & 1);
    digitalWrite(PIN_RIGHT, (inputs >> 1) & 1);
    digitalWrite(PIN_UP, (inputs >> 2) & 1);
    digitalWrite(PIN_DOWN, (inputs >> 3) & 1);
    digitalWrite(PIN_A, (inputs >> 4) & 1);
    digitalWrite(PIN_B, (inputs >> 5) & 1);
}

uint32_t consumeDelay() {
    return consumeDelayedMillis();
}

}

uint32_t getSeed() {
    return seed;
}
#endif
