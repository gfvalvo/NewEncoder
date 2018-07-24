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
	~NewEncoder();
	bool begin();
	int16_t getValue();
	int16_t operator =(int16_t val) {
		_currentValue = val;
		return _currentValue;
	}

private:
	void pinChangeHandler(uint8_t pin);
	uint8_t _aPin, _bPin;
	volatile int16_t _minValue, _maxValue;
	int16_t _interruptA, _interruptB;
	volatile uint8_t aPinValue, bPinValue;
	volatile uint8_t _currentState;
	volatile int16_t _currentValue;
	volatile IO_REG_TYPE * aPin_register;
	volatile IO_REG_TYPE * bPin_register;
	IO_REG_TYPE aPin_bitmask;
	IO_REG_TYPE bPin_bitmask;

	struct isrStruct {
		void (*pinChangeIsr)();
		NewEncoder *assignedEncoder;
	};

	static isrStruct isrTable[];
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

#define PIN_CHANGE_ISR(interruptNumber)  \
    static void interrupt ## interruptNumber ## ChangeIsr(void) {  \
	  isrTable[interruptNumber].assignedEncoder->pinChangeHandler(interruptNumber);  \
	}

#if CORE_NUM_INTERRUPT > 0
	PIN_CHANGE_ISR(0)
	#endif
#if CORE_NUM_INTERRUPT > 1
	PIN_CHANGE_ISR(1)
	#endif
#if CORE_NUM_INTERRUPT > 2
	PIN_CHANGE_ISR(2)
	#endif
#if CORE_NUM_INTERRUPT > 3
	PIN_CHANGE_ISR(3)
	#endif
#if CORE_NUM_INTERRUPT > 4
	PIN_CHANGE_ISR(4)
	#endif
#if CORE_NUM_INTERRUPT > 5
	PIN_CHANGE_ISR(5)
	#endif
#if CORE_NUM_INTERRUPT > 6
	PIN_CHANGE_ISR(6)
	#endif
#if CORE_NUM_INTERRUPT > 7
	PIN_CHANGE_ISR(7)
	#endif
#if CORE_NUM_INTERRUPT > 8
	PIN_CHANGE_ISR(8)
	#endif
#if CORE_NUM_INTERRUPT > 9
	PIN_CHANGE_ISR(9)
	#endif
#if CORE_NUM_INTERRUPT > 10
	PIN_CHANGE_ISR(10)
	#endif
#if CORE_NUM_INTERRUPT > 11
	PIN_CHANGE_ISR(11)
	#endif
#if CORE_NUM_INTERRUPT > 12
	PIN_CHANGE_ISR(12)
	#endif
#if CORE_NUM_INTERRUPT > 13
	PIN_CHANGE_ISR(13)
	#endif
#if CORE_NUM_INTERRUPT > 14
	PIN_CHANGE_ISR(14)
	#endif
#if CORE_NUM_INTERRUPT > 15
	PIN_CHANGE_ISR(15)
	#endif
#if CORE_NUM_INTERRUPT > 16
	PIN_CHANGE_ISR(16)
	#endif
#if CORE_NUM_INTERRUPT > 17
	PIN_CHANGE_ISR(17)
	#endif
#if CORE_NUM_INTERRUPT > 18
	PIN_CHANGE_ISR(18)
	#endif
#if CORE_NUM_INTERRUPT > 19
	PIN_CHANGE_ISR(19)
	#endif
#if CORE_NUM_INTERRUPT > 20
	PIN_CHANGE_ISR(20)
	#endif
#if CORE_NUM_INTERRUPT > 21
	PIN_CHANGE_ISR(21)
	#endif
#if CORE_NUM_INTERRUPT > 22
	PIN_CHANGE_ISR(22)
	#endif
#if CORE_NUM_INTERRUPT > 23
	PIN_CHANGE_ISR(23)
	#endif
#if CORE_NUM_INTERRUPT > 24
	PIN_CHANGE_ISR(24)
	#endif
#if CORE_NUM_INTERRUPT > 25
	PIN_CHANGE_ISR(25)
	#endif
#if CORE_NUM_INTERRUPT > 26
	PIN_CHANGE_ISR(26)
	#endif
#if CORE_NUM_INTERRUPT > 27
	PIN_CHANGE_ISR(27)
	#endif
#if CORE_NUM_INTERRUPT > 28
	PIN_CHANGE_ISR(28)
	#endif
#if CORE_NUM_INTERRUPT > 29
	PIN_CHANGE_ISR(29)
	#endif
#if CORE_NUM_INTERRUPT > 30
	PIN_CHANGE_ISR(30)
	#endif
#if CORE_NUM_INTERRUPT > 31
	PIN_CHANGE_ISR(31)
	#endif
#if CORE_NUM_INTERRUPT > 32
	PIN_CHANGE_ISR(32)
	#endif
#if CORE_NUM_INTERRUPT > 33
	PIN_CHANGE_ISR(33)
	#endif
#if CORE_NUM_INTERRUPT > 34
	PIN_CHANGE_ISR(34)
#endif
#if CORE_NUM_INTERRUPT > 35
	PIN_CHANGE_ISR(35)
#endif
#if CORE_NUM_INTERRUPT > 36
	PIN_CHANGE_ISR(36)
#endif
#if CORE_NUM_INTERRUPT > 37
	PIN_CHANGE_ISR(37)
#endif
#if CORE_NUM_INTERRUPT > 38
	PIN_CHANGE_ISR(38)
#endif
#if CORE_NUM_INTERRUPT > 39
	PIN_CHANGE_ISR(39)
#endif
#if CORE_NUM_INTERRUPT > 40
	PIN_CHANGE_ISR(40)
#endif
#if CORE_NUM_INTERRUPT > 41
	PIN_CHANGE_ISR(41)
#endif
#if CORE_NUM_INTERRUPT > 42
	PIN_CHANGE_ISR(42)
#endif
#if CORE_NUM_INTERRUPT > 43
	PIN_CHANGE_ISR(43)
#endif
#if CORE_NUM_INTERRUPT > 44
	PIN_CHANGE_ISR(44)
#endif
#if CORE_NUM_INTERRUPT > 45
	PIN_CHANGE_ISR(45)
#endif
#if CORE_NUM_INTERRUPT > 46
	PIN_CHANGE_ISR(46)
#endif
#if CORE_NUM_INTERRUPT > 47
	PIN_CHANGE_ISR(47)
#endif
#if CORE_NUM_INTERRUPT > 48
	PIN_CHANGE_ISR(48)
#endif
#if CORE_NUM_INTERRUPT > 49
	PIN_CHANGE_ISR(49)
#endif
#if CORE_NUM_INTERRUPT > 50
	PIN_CHANGE_ISR(50)
#endif
#if CORE_NUM_INTERRUPT > 51
	PIN_CHANGE_ISR(51)
#endif
#if CORE_NUM_INTERRUPT > 52
	PIN_CHANGE_ISR(52)
#endif
#if CORE_NUM_INTERRUPT > 53
	PIN_CHANGE_ISR(53)
#endif
#if CORE_NUM_INTERRUPT > 54
	PIN_CHANGE_ISR(54)
#endif
#if CORE_NUM_INTERRUPT > 55
	PIN_CHANGE_ISR(55)
#endif
#if CORE_NUM_INTERRUPT > 56
	PIN_CHANGE_ISR(56)
#endif
#if CORE_NUM_INTERRUPT > 57
	PIN_CHANGE_ISR(57)
#endif
#if CORE_NUM_INTERRUPT > 58
	PIN_CHANGE_ISR(58)
#endif
#if CORE_NUM_INTERRUPT > 59
	PIN_CHANGE_ISR(59)
#endif
#if CORE_NUM_INTERRUPT > 60
	PIN_CHANGE_ISR(60)
#endif
#if CORE_NUM_INTERRUPT > 61
	PIN_CHANGE_ISR(61)
#endif
#if CORE_NUM_INTERRUPT > 62
	PIN_CHANGE_ISR(62)
#endif
#if CORE_NUM_INTERRUPT > 63
	PIN_CHANGE_ISR(63)
#endif
};


#endif /* NEWENCODER_H_ */
