#include "input.h"
#include "Arduino.h"

uint8_t cur_input = 0xff;

uint8_t read_input() {
	if (cur_input == 0xff) {
		Serial.begin(9600);
		cur_input = 0;
	}
	
	while (Serial.available()) {
		cur_input = Serial.read();
	}
	
	return cur_input;
}
