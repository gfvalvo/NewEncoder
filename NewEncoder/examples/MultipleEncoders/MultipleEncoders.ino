#include "Arduino.h"
#include "NewEncoder.h"

NewEncoder encoders[] = { { 0, 1, -20, 20 }, { 20, 21, 0, 50 }, { 5, 6, -25, 0 }, { 11, 12, -10, 25 } };
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

	for (uint8_t index = 0; index < numEncoders; index++) {
		currentValue = encoders[index];
		if (currentValue != prevEncoderValue[index]) {
			Serial.print(F("Encoder"));
			Serial.print(index);
			Serial.print(F(": "));
			Serial.println(currentValue);
			prevEncoderValue[index] = currentValue;
		}
	}
}
