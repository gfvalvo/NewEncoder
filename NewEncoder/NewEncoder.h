/*
 * NewEncoder.h
 *
 *  Created on: Jul 18, 2018
 *      Author: TR001221
 */

#ifndef NEWENCODER_H_
#define NEWENCODER_H_

#include <Arduino.h>

//#define DEBUG_MODE


#define AF 0b00
#define AR 0b01
#define BF 0b10
#define BR 0b11

#ifdef DEBUG_MODE
extern const uint8_t fifoSize;
extern const uint8_t fifoMask;
extern volatile bool overflow;
extern volatile uint8_t fillLevel;
extern volatile uint8_t fifo[];
#endif


class NewEncoder {
public:
	NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue);
	~NewEncoder();
	bool begin();
	int16_t getValue();

private:
	void pinChangeHandler(uint8_t pin);
	uint8_t _aPin, _bPin;
	volatile int16_t _minValue, _maxValue;
	int16_t _interruptA, _interruptB;
	volatile uint8_t aPinValue, bPinValue;
	volatile uint8_t _currentState;
	volatile int16_t _currentValue;

	struct isrStruct {
		void (*pinChangeIsr)();
		NewEncoder *assignedEncoder;
	};

	static void interrupt00ChangeIsr();
	static void interrupt01ChangeIsr();
	static void interrupt02ChangeIsr();
	static void interrupt03ChangeIsr();
	static void interrupt04ChangeIsr();
	static void interrupt05ChangeIsr();

	static isrStruct isrTable[];
	static const uint8_t numInterrupts;
	static const uint8_t stateMask = 0b00000111;
	static const uint8_t deltaMask = 0b00011000;
	static const uint8_t incrementDelta = 0b00001000;
	static const uint8_t decrementDelta = 0b00010000;
	static const uint8_t startState = 0b011;
	static const uint8_t cwState1 = 0b010;
	static const uint8_t cwState2 = 0b000;
	static const uint8_t cwState3 = 0b001;
	static const uint8_t ccwState1 = 0b101;
	static const uint8_t ccwState2 = 0b100;
	static const uint8_t ccwState3 = 0b110;
	static const uint8_t transistionTable[][4];
};

#endif /* NEWENCODER_H_ */
