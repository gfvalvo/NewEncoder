// Do not remove the include below
#include "TestEncoder.h"
#include "NewEncoder.h"

NewEncoder encoder1(22, 23, -20, 20);

const uint8_t resetPin = 0;
uint8_t lastResetReading;
int16_t previousValue;
void setup() {
	Serial.begin(115200);
	delay(2000);
	Serial.println("Starting");
	pinMode(resetPin, INPUT_PULLUP);
	lastResetReading = digitalRead(resetPin);
	if (!encoder1.begin()) {
		Serial.println("Encoder Failed to Start");
		while (1) {
		}
	}
	Serial.println("Encoder Started Successfully");
	previousValue = encoder1;
	Serial.println(previousValue);
}

void loop() {
	int16_t currentValue;
	uint8_t currentResetReading;
	currentValue = encoder1;
	if (currentValue != previousValue) {
		Serial.println(currentValue);
		previousValue = currentValue;
	}

	currentResetReading = digitalRead(resetPin);
	if ((currentResetReading != lastResetReading)
			&& (currentResetReading == 0)) {
		encoder1 = 0;
		currentValue = encoder1;
		Serial.println(currentValue);
		previousValue = currentValue;
	}
	lastResetReading = currentResetReading;

#ifdef DEBUG_MODE
	static uint8_t readPointer = 0;
	uint8_t localTransistionType;
	if (overflow) {
		noInterrupts()
		;
		overflow = false;
		interrupts()
		;
		Serial.println("Fifo Overflow");
	}
	if (fillLevel > 0) {
		noInterrupts()
		;
		localTransistionType = fifo[readPointer++];
		fillLevel--;
		interrupts()
		;
		readPointer &= fifoMask;
		switch (localTransistionType) {
		case AF:
			Serial.println("AF");
			break;

		case AR:
			Serial.println("AR");
			break;

		case BF:
			Serial.println("BF");
			break;

		case BR:
			Serial.println("BR");
			break;

		default:
		break;
		}
	}
#endif
}
