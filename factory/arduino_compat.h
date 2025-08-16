#ifdef WEB

#include <cstdint>

#define INPUT 1
#define OUTPUT 0
#define INPUT_PULLUP 2
#define LOW 0
#define HIGH 1
#define PROGMEM

#define ARM_DWT_CYCCNT getSeed();

uint32_t getSeed();

void pinMode(int pin, int mode);
int digitalRead(int pin);
void digitalWrite(int pin, int level);
int analogRead(int pin);
uint32_t millis();
void delay(uint32_t ms);
void delayNanoseconds(uint32_t ns);

void randomSeed(uint32_t seed);
int32_t random(int32_t max);
int32_t random(int32_t min, int32_t max);

class SerialClass {
public:
    void println(const char *s);
};
extern SerialClass Serial;

uint32_t consumeDelayedMillis();

#else

#include "Arduino.h"

#endif
