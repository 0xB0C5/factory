#include "arduino_compat.h"
#ifdef WEB

#define PIN_COUNT 32
int pins[PIN_COUNT] = {};

uint32_t ms = 0;
uint32_t pending_ms = 0;

void pinMode(int pin, int mode) {
}

void delay(uint32_t millis) {
    ms += millis;
    pending_ms += millis;
}

void delayNanoseconds(uint32_t nanos) {
}

uint32_t millis() {
    return ms;
}


uint32_t rand = 0;
void randomSeed(uint32_t seed) {
    rand = seed ? seed : 1;
}

uint32_t random() {
	rand ^= rand << 13;
	rand ^= rand >> 17;
	rand ^= rand << 5;
    return rand;
}

int32_t random(int32_t max) {
    return (int32_t) ((((int64_t)random())*max) >> 32);
}

int32_t random(int32_t min, int32_t max) {
    return min + random(max - min);
}

int digitalRead(int pin) {
    return pins[pin];
}

int analogRead(int pin) {
    return pins[pin];
}

void digitalWrite(int pin, int level) {
    pins[pin] = level;
}

uint32_t consumeDelayedMillis() {
    uint32_t res = pending_ms;
    pending_ms = 0;
    return res;
}

#endif
