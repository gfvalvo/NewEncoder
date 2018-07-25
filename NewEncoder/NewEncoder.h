/*
 * NewEncoder.h
 *
 *  Created on: Jul 18, 2018
 *      Author: TR001221
 */
#ifndef NEWENCODER_H_
#define NEWENCODER_H_

#include <Arduino.h>
#include "utility\interrupt_pins.h"
#include "utility\direct_pin_read.h"

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
	NewEncoder();
	~NewEncoder();
	bool begin(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue);
	bool begin();
	void end();
	int16_t setValue(int16_t);
	int16_t getValue();
	int16_t operator =(int16_t val);
	operator int16_t() const;

private:
	void pinChangeHandler(uint8_t index);
	void aPinChange();
	void bPinChange();
	bool active = false;
	uint8_t _aPin = 0, _bPin = 0;
	volatile int16_t _minValue = 0, _maxValue = 0;
	int16_t _interruptA = -1, _interruptB = -1;
	volatile uint8_t _aPinValue, _bPinValue;
	volatile uint8_t _currentState;
	volatile int16_t _currentValue;
	volatile IO_REG_TYPE * _aPin_register;
	volatile IO_REG_TYPE * _bPin_register;
	volatile IO_REG_TYPE _aPin_bitmask;
	volatile IO_REG_TYPE _bPin_bitmask;

	static uint8_t _numEncoders;
	static const uint8_t _stateMask = 0b00000111;
	static const uint8_t _deltaMask = 0b00011000;
	static const uint8_t _incrementDelta = 0b00001000;
	static const uint8_t _decrementDelta = 0b00010000;
	static const uint8_t _startState = 0b011;
	static const uint8_t _cwState1 = 0b010;
	static const uint8_t _cwState2 = 0b000;
	static const uint8_t _cwState3 = 0b001;
	static const uint8_t _ccwState1 = 0b101;
	static const uint8_t _ccwState2 = 0b100;
	static const uint8_t _ccwState3 = 0b110;
	static const uint8_t _transistionTable[][4];
	static const uint8_t _maxNumEncoders = CORE_NUM_INTERRUPT / 2;
	static NewEncoder *_encoderTable[];

	static void aPinIsr();
	static void bPinIsr();
};

#endif /* NEWENCODER_H_ */
