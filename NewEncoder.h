/*
 * NewEncoder.h
 */
#ifndef NEWENCODER_H_
#define NEWENCODER_H_

#include <Arduino.h>
#include "utility\interrupt_pins.h"
#include "utility\direct_pin_read.h"

class NewEncoder {
public:
	NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue);
	NewEncoder();
	~NewEncoder();
	bool begin();
	void configure(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue);
	void end();
	bool enabled();
	int16_t setValue(int16_t);
	int16_t getValue();
	int16_t operator =(int16_t val);
	operator int16_t() const;
	bool upClick();
	bool downClick();

private:
	void pinChangeHandler(uint8_t index);
	void aPinChange();
	void bPinChange();
	bool active = false;
	bool configured = false;
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
	volatile bool clickUp = false;
	volatile bool clickDown = false;

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
	static const uint8_t aPinFalling = 0b00;
	static const uint8_t aPinRising = 0b01;
	static const uint8_t bPinFalling = 0b10;
	static const uint8_t bPinRising = 0b11;
	static NewEncoder *_encoderTable[];

	static void aPinIsr();
	static void bPinIsr();
};

#endif /* NEWENCODER_H_ */
