#include "Arduino.h"
#include "NewEncoder.h"

// Adjust number of encoders and pin assignments for particular processor. These work for Teensy 3.2. See README for meaning of constructor arguments.
// Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
// Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.adafruit.com/product/377
NewEncoder encoders[] = {
		{ 0, 1, -20, 20, 0, FULL_PULSE },
		{ 20, 21, 0, 50, 25, FULL_PULSE },
		{ 5, 6, -25, 0, -13, FULL_PULSE },
		{ 11, 12, -10, 25, 8, FULL_PULSE }
};

const uint8_t numEncoders = sizeof(encoders) / sizeof(NewEncoder);
int16_t prevEncoderValue[numEncoders];

void setup() {
	int16_t value;

	Serial.begin(115200);
	delay(2000);
	Serial.println(F("Starting"));

	for (uint8_t index = 0; index < numEncoders; index++) {
		if (!encoders[index].begin()) {
			Serial.print(F("Encoder: "));
			Serial.print(index);
			Serial.println(
					F(" Failed to Start. Check pin assignments and available interrupts. Aborting."));
			while (1) {
				yield();
			}
		} else {
			value = encoders[index];
			prevEncoderValue[index] = value;
			Serial.print(F("Encoder: "));
			Serial.print(index);
			Serial.print(F(" Successfully Started at value = "));
			Serial.println(value);
		}
	}
}

void loop() {
	int16_t currentValue;
	bool up, down;

	for (uint8_t index = 0; index < numEncoders; index++) {
		up = encoders[index].upClick();
		down = encoders[index].downClick();
		if (up || down) {
			currentValue = encoders[index];
			if (currentValue != prevEncoderValue[index]) {
				Serial.print(F("Encoder"));
				Serial.print(index);
				Serial.print(F(": "));
				Serial.println(currentValue);
				prevEncoderValue[index] = currentValue;
			} else if (up) {
				Serial.print(F("Encoder"));
				Serial.print(index);
				Serial.println(F(": At upper limit."));
			} else {
				Serial.print(F("Encoder"));
				Serial.print(index);
				Serial.println(F(": At lower limit."));
			}
		}
	}
}
