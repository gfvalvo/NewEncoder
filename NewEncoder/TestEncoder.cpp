// Do not remove the include below
#include "TestEncoder.h"
#include "NewEncoder.h"

NewEncoder encoder1(0, 1, -20, 20);

int16_t previousValue;
void setup() {
	Serial.begin(115200);
	delay(2000);
	Serial.println("Starting");
	if (!encoder1.begin()) {
		Serial.println("Encoder Failed to Start");
		while (1) {
		}
	}
	Serial.println("Encoder Started Successfully");
	previousValue = encoder1.getValue();
	Serial.println(previousValue);
}

void loop() {
	int16_t currentValue;
	currentValue = encoder1.getValue();
	if (currentValue != previousValue) {
		Serial.println(currentValue);
		previousValue = currentValue;
	}

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
