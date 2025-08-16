
#include "battery.h"
#include "arduino_compat.h"

#define PIN_BATTERY 14

#define BATTERY_MIN_VALUE 723
#define BATTERY_MAX_VALUE 942

void battery_init() {
	pinMode(PIN_BATTERY, INPUT);
}

uint8_t battery_percent() {
	uint16_t value = analogRead(PIN_BATTERY);

	//Serial.print("BATTERY VALUE: ");
	//Serial.println(value);

	if (value < BATTERY_MIN_VALUE) value = BATTERY_MIN_VALUE;
	if (value > BATTERY_MAX_VALUE) value = BATTERY_MAX_VALUE;

	uint8_t percent = (uint8_t)(value - BATTERY_MIN_VALUE) * 100 / (BATTERY_MAX_VALUE - BATTERY_MIN_VALUE);

	//Serial.print("BATTERY PERCENT: ");
	//Serial.println(percent);

	return percent;
}
