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


NewEncoder::isrStruct NewEncoder::isrTable[] = {
		{ interrupt00ChangeIsr, nullptr },
		{ interrupt01ChangeIsr, nullptr },
		{ interrupt02ChangeIsr, nullptr },
		{ interrupt03ChangeIsr, nullptr },
		{ interrupt04ChangeIsr, nullptr },
		{ interrupt05ChangeIsr, nullptr } };

const uint8_t NewEncoder::numInterrupts = sizeof(isrTable) / sizeof(isrTable[0]);

const uint8_t NewEncoder::transistionTable[][4] = {
		{ cwState2, cwState3, cwState2, cwState1 }, // cwState2 = 0b000
		{ cwState2, cwState3, cwState3, startState | incrementDelta }, // cwState3 = 0b001
		{ cwState1, startState, cwState2, cwState1 },  // cwState1 = 0b010
		{ cwState1, startState, ccwState1, startState }, // startState = 0b011
		{ ccwState2, ccwState1, ccwState2, ccwState3 },  // ccwState2 = 0b100
		{ ccwState2, ccwState1, ccwState1, startState },  // ccwState1 = 0b101
		{ ccwState3, startState | decrementDelta, ccwState2, ccwState3 },  // ccwState3 = 0b110
		{ startState, startState, startState, startState } // illegal state should never be in it
};

NewEncoder::NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue) :
		_aPin(aPin), _bPin(bPin), _minValue(minValue), _maxValue(maxValue),
				_interruptA(digitalPinToInterrupt(_aPin)), _interruptB(digitalPinToInterrupt(_bPin)) {
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
	aPinValue = digitalRead(_aPin);
	aPinValue = digitalRead(_aPin);
	bPinValue = digitalRead(_bPin);
	bPinValue = digitalRead(_bPin);
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
		bPinValue = digitalRead(_bPin);
		if (bPinValue) {
			index = BR;
		} else {
			index = BF;
		}
	} else {
		aPinValue = digitalRead(_aPin);
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

void NewEncoder::interrupt00ChangeIsr() {
	isrTable[00].assignedEncoder->pinChangeHandler(00);
}

void NewEncoder::interrupt01ChangeIsr() {
	isrTable[01].assignedEncoder->pinChangeHandler(01);
}

void NewEncoder::interrupt02ChangeIsr() {
	isrTable[02].assignedEncoder->pinChangeHandler(02);
}

void NewEncoder::interrupt03ChangeIsr() {
	isrTable[03].assignedEncoder->pinChangeHandler(03);
}

void NewEncoder::interrupt04ChangeIsr() {
	isrTable[04].assignedEncoder->pinChangeHandler(04);
}

void NewEncoder::interrupt05ChangeIsr() {
	isrTable[04].assignedEncoder->pinChangeHandler(04);
}
