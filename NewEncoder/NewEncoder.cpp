/*
 * NewEncoder.cpp
 *
 *  Created on: Jul 18, 2018
 *      Author: TR001221
 */

#include "NewEncoder.h"

#ifdef DEBUG_MODE
static const uint8_t log2FifoSize = 7;
const uint8_t fifoSize = 1 << log2FifoSize;
const uint8_t fifoMask = fifoSize - 1;
volatile bool overflow = false;
volatile uint8_t fillLevel = 0;
volatile uint8_t fifo[fifoSize];

static volatile uint8_t writePointer = 0;
#endif

const uint8_t NewEncoder::transistionTable[][4] = {
		{ cwState2, cwState3, cwState2, cwState1 }, // cwState2 = 0b000
		{ cwState2, cwState3, cwState3, startState | incrementDelta }, // cwState3 = 0b001
		{ cwState1, startState, cwState2, cwState1 },  // cwState1 = 0b010
		{ cwState1, startState, ccwState1, startState }, // startState = 0b011
		{ ccwState2, ccwState1, ccwState2, ccwState3 },  // ccwState2 = 0b100
		{ ccwState2, ccwState1, ccwState1, startState },  // ccwState1 = 0b101
		{ ccwState3, startState | decrementDelta, ccwState2, ccwState3 }, // ccwState3 = 0b110
		{ startState, startState, startState, startState } // illegal state should never be in it
};

NewEncoder::NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue) :
		_aPin(aPin), _bPin(bPin), _minValue(minValue), _maxValue(maxValue),
				_interruptA(digitalPinToInterrupt(_aPin)), _interruptB(
						digitalPinToInterrupt(_bPin)) {
	aPin_register = PIN_TO_BASEREG(aPin);
	bPin_register = PIN_TO_BASEREG(bPin);
	aPin_bitmask = PIN_TO_BITMASK(aPin);
	bPin_bitmask = PIN_TO_BITMASK(bPin);
}

NewEncoder::~NewEncoder() {
	if (_interruptA >= 0) {
		isrTable[_interruptA].assignedEncoder = nullptr;
		detachInterrupt(_interruptA);
	}

	if (_interruptB >= 0) {
		isrTable[_interruptB].assignedEncoder = nullptr;
		detachInterrupt(_interruptB);
	}
}

bool NewEncoder::begin() {
	if (_aPin == _bPin) {
		return false;
	}

	if (_interruptA == _interruptB) {
		return false;
	}

	if (_interruptA < 0) {
		return false;
	}

	if (_interruptB < 0) {
		return false;
	}

	isrTable[_interruptA].assignedEncoder = this;
	isrTable[_interruptB].assignedEncoder = this;

	pinMode(_aPin, INPUT_PULLUP);
	pinMode(_bPin, INPUT_PULLUP);
	aPinValue = DIRECT_PIN_READ(aPin_register, aPin_bitmask);
	bPinValue = DIRECT_PIN_READ(bPin_register, bPin_bitmask);
	_currentState = (bPinValue << 1) | aPinValue;
	_currentValue = (_minValue + _maxValue) / 2;

	attachInterrupt(_interruptA, isrTable[_interruptA].pinChangeIsr, CHANGE);
	attachInterrupt(_interruptB, isrTable[_interruptB].pinChangeIsr, CHANGE);

	return true;
}

int16_t NewEncoder::getValue() {
	return _currentValue;
}

void NewEncoder::pinChangeHandler(uint8_t interruptNumber) {
	uint8_t newState, index;

	if (interruptNumber == _interruptB) {
		bPinValue = DIRECT_PIN_READ(bPin_register, bPin_bitmask);
		if (bPinValue) {
			index = BR;
		} else {
			index = BF;
		}
	} else {
		aPinValue = DIRECT_PIN_READ(aPin_register, aPin_bitmask);
		if (aPinValue) {
			index = AR;
		} else {
			index = AF;
		}
	}

#ifdef DEBUG_MODE
	if (fillLevel < fifoSize) {
		fillLevel++;
		fifo[writePointer++] = index;
		writePointer &= fifoMask;
	} else {
		overflow = true;
	}
#endif

	newState = NewEncoder::transistionTable[_currentState][index];
	_currentState = newState & stateMask;
	if ((newState & deltaMask) == incrementDelta) {
		if (_currentValue < _maxValue) {
			_currentValue++;
		}
	} else if ((newState & deltaMask) == decrementDelta) {
		if (_currentValue > _minValue) {
			_currentValue--;
		}
	}
}

NewEncoder::isrStruct NewEncoder::isrTable[] = {
		#if CORE_NUM_INTERRUPT > 0
		{ interrupt0ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 1
		, { interrupt1ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 2
		, { interrupt2ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 3
		, { interrupt3ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 4
		, { interrupt0ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 5
		, { interrupt5ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 6
		, { interrupt6ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 7
		, { interrupt7ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 8
		, { interrupt8ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 9
		, { interrupt9ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 10
		, { interrupt10ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 11
		, { interrupt11ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 12
		, { interrupt12ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 13
		, { interrupt13ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 14
		, { interrupt14ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 15
		, { interrupt15ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 16
		, { interrupt16ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 17
		, { interrupt17ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 18
		, { interrupt18ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 19
		, { interrupt19ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 20
		, { interrupt20ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 21
		, { interrupt21ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 22
		, { interrupt22ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 23
		, { interrupt23ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 24
		, { interrupt24ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 25
		, { interrupt25ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 26
		, { interrupt26ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 27
		, { interrupt27ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 28
		, { interrupt28ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 29
		, { interrupt29ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 30
		, { interrupt30ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 31
		, { interrupt30ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 32
		, { interrupt32ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 33
		, { interrupt33ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 34
		, { interrupt34ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 35
		, { interrupt35ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 36
		, { interrupt36ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 37
		, { interrupt37ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 38
		, { interrupt38ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 39
		, { interrupt39ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 40
		, { interrupt40ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 41
		, { interrupt41ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 42
		, { interrupt42ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 43
		, { interrupt43ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 44
		, { interrupt44ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 45
		, { interrupt45ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 46
		, { interrupt46ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 47
		, { interrupt47ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 48
		, { interrupt48ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 49
		, { interrupt49ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 50
		, { interrupt50ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 51
		, { interrupt51ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 52
		, { interrupt52ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 53
		, { interrupt53ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 54
		, { interrupt54ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 55
		, { interrupt55ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 56
		, { interrupt56ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 57
		, { interrupt57ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 58
		, { interrupt58ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 59
		, { interrupt59ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 60
		, { interrupt60ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 61
		, { interrupt61ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 62
		, { interrupt62ChangeIsr, nullptr }
		#endif
#if CORE_NUM_INTERRUPT > 63
		, { interrupt63ChangeIsr, nullptr }
#endif
		};

