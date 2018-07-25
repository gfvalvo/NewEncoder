// Do not remove the include below
#include "TestEncoder.h"
#include "NewEncoder.h"

const uint8_t resetPin = 0;
uint8_t lastResetSwitchState;
int16_t encoder0PrevValue, encoder1PrevValue;

NewEncoder *dummyEncoder0, *encoder0, *encoder1;
void setup() {
	Serial.begin(115200);
	delay(2000);
	Serial.println("Starting");
	pinMode(resetPin, INPUT_PULLUP);
	lastResetSwitchState = digitalRead(resetPin);

	dummyEncoder0 = new NewEncoder();
	encoder0 = new NewEncoder();
	encoder1 = new NewEncoder();

	if (!dummyEncoder0->begin(0, 1, -20, 20)) {
		Serial.println("dummyEncoder0 Failed to Start");
		while (1) {
		}
	}
	Serial.println("dummyEncoder0 Started Successfully");

	if (!encoder1->begin(11, 12, -20, 20)) {
		Serial.println("encoder1 Failed to Start");
		while (1) {
		}
	}
	Serial.println("encoder1 Started Successfully");
	encoder1PrevValue = *encoder1;
	Serial.print("encoder1: ");
	Serial.println(encoder1PrevValue);

	if (!encoder0->begin(22, 23, -20, 20)) {
		Serial.println("encoder0 Failed to Start");
		while (1) {
		}
	}
	Serial.println("encoder0 Started Successfully");
	encoder0PrevValue = *encoder0;
	Serial.print("encoder0: ");
	Serial.println(encoder0PrevValue);
}

void loop() {
	int16_t currentValue;
	uint8_t currentResetSwitchState;

	currentValue = *encoder0;
	if (currentValue != encoder0PrevValue) {
		Serial.print("encoder0: ");
		Serial.println(currentValue);
		encoder0PrevValue = currentValue;
	}

	if (encoder1->enabled()) {
		currentValue = *encoder1;
		if (currentValue != encoder1PrevValue) {
			Serial.print("encoder1: ");
			Serial.println(currentValue);
			encoder1PrevValue = currentValue;
		}
	}

	currentResetSwitchState = digitalRead(resetPin);
	if ((currentResetSwitchState != lastResetSwitchState)
			&& (currentResetSwitchState == 0)) {
		*encoder0 = 0;
		currentValue = *encoder0;
		Serial.print("encoder0: ");
		Serial.println(currentValue);
		encoder0PrevValue = currentValue;
		if (dummyEncoder0) {
			delete dummyEncoder0;
			dummyEncoder0 = nullptr;
			Serial.println("dummyEncoder0 deleted");
		} else if (encoder1) {
			delete encoder1;
			encoder1 = nullptr;
			Serial.println("encoder1 deleted");
		}
	}
	lastResetSwitchState = currentResetSwitchState;

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
