/*
 * NewEncoder.cpp
 *
 *  Created on: Jul 18, 2018
 *      Author: TR001221
 */

#include "NewEncoder.h"

elapsedMillis dog;

#ifdef DEBUG_MODE
static const uint8_t log2FifoSize = 7;
const uint8_t fifoSize = 1 << log2FifoSize;
const uint8_t fifoMask = fifoSize - 1;
volatile bool overflow = false;
volatile uint8_t fillLevel = 0;
volatile uint8_t fifo[fifoSize];

static volatile uint8_t writePointer = 0;
#endif

const uint8_t NewEncoder::_transistionTable[][4] = {
		{ _cwState2, _cwState3, _cwState2, _cwState1 }, // cwState2 = 0b000
		{ _cwState2, _cwState3, _cwState3, _startState | _incrementDelta }, // cwState3 = 0b001
		{ _cwState1, _startState, _cwState2, _cwState1 },  // cwState1 = 0b010
		{ _cwState1, _startState, _ccwState1, _startState }, // startState = 0b011
		{ _ccwState2, _ccwState1, _ccwState2, _ccwState3 }, // ccwState2 = 0b100
		{ _ccwState2, _ccwState1, _ccwState1, _startState }, // ccwState1 = 0b101
		{ _ccwState3, _startState | _decrementDelta, _ccwState2, _ccwState3 }, // ccwState3 = 0b110
		{ _startState, _startState, _startState, _startState } // illegal state should never be in it
};

uint8_t NewEncoder::_numEncoders = 0;
NewEncoder *NewEncoder::_encoderTable[_maxNumEncoders];

NewEncoder::NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue) {
	_aPin = aPin;
	_bPin = bPin;
	_minValue = minValue;
	_maxValue = maxValue;
	_interruptA = digitalPinToInterrupt(_aPin);
	_interruptB = digitalPinToInterrupt(_bPin);
	_aPin_register = PIN_TO_BASEREG(aPin);
	_bPin_register = PIN_TO_BASEREG(bPin);
	_aPin_bitmask = PIN_TO_BITMASK(aPin);
	_bPin_bitmask = PIN_TO_BITMASK(bPin);
	active = false;
}

NewEncoder::NewEncoder() {
	active = false;
}

NewEncoder::~NewEncoder() {
	end();
}

void NewEncoder::end() {
	uint8_t encoderIndex;
	if (!active) {
		return;
	}
	active = false;
	detachInterrupt(_interruptA);
	detachInterrupt(_interruptB);
	bool found = false;
	for (encoderIndex = 0; encoderIndex < _numEncoders; encoderIndex++) {
		if (_encoderTable[encoderIndex] == this) {
			found = true;
			break;
		}
	}
	if (!found) {
		return;
	}
	_numEncoders--;
	if (_numEncoders == 0) {
		return;
	}
	for (uint8_t j = encoderIndex; j < _numEncoders; j++) {
		_encoderTable[j] = _encoderTable[j + 1];
	}
}

bool NewEncoder::begin(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue) {
	if (active) {
		return false;
	}
	_aPin = aPin;
	_bPin = bPin;
	_minValue = minValue;
	_maxValue = maxValue;
	_interruptA = digitalPinToInterrupt(_aPin);
	_interruptB = digitalPinToInterrupt(_bPin);
	_aPin_register = PIN_TO_BASEREG(aPin);
	_bPin_register = PIN_TO_BASEREG(bPin);
	_aPin_bitmask = PIN_TO_BITMASK(aPin);
	_bPin_bitmask = PIN_TO_BITMASK(bPin);
	return begin();
}

bool NewEncoder::begin() {
	if (active) {
		return false;
	}
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

	if (_minValue == _maxValue) {
		return false;
	}

	if (_numEncoders == 0) {
		for (uint8_t index = 0; index < _maxNumEncoders; index++) {
			_encoderTable[index] = nullptr;
		}
	}
	_encoderTable[_numEncoders++] = this;

	pinMode(_aPin, INPUT_PULLUP);
	pinMode(_bPin, INPUT_PULLUP);
	_aPinValue = DIRECT_PIN_READ(_aPin_register, _aPin_bitmask);
	_bPinValue = DIRECT_PIN_READ(_bPin_register, _bPin_bitmask);
	_currentState = (_bPinValue << 1) | _aPinValue;
	_currentValue = (_minValue + _maxValue) / 2;

	attachInterrupt(_interruptA, aPinIsr, CHANGE);
	attachInterrupt(_interruptB, bPinIsr, CHANGE);
	active = true;
	return true;
}

int16_t NewEncoder::setValue(int16_t val) {
	if (val < _minValue) {
		val = _minValue;
	} else if (val > _maxValue) {
		val = _maxValue;
	}
#if defined(__AVR__)
	noInterrupts();  // 16-bit access not atomic on 8-bit processor
#endif
	_currentValue = val;
#if defined(__AVR__)
	interrupts();
#endif
	return val;
}

int16_t NewEncoder::operator =(int16_t val) {
	return setValue(val);
}

int16_t NewEncoder::getValue() {
#if defined(__AVR__)
	int16_t val;
	noInterrupts();  // 16-bit access not atomic on 8-bit processor
	val = _currentValue;
	interrupts();
	return val;
#else
	return _currentValue;
#endif
}

NewEncoder::operator int16_t() const {
#if defined(__AVR__)
	int16_t val;
	noInterrupts();  // 16-bit access not atomic on 8-bit processor
	val = _currentValue;
	interrupts();
	return val;
#else
	return _currentValue;
#endif
}

void NewEncoder::aPinChange() {
	uint8_t newPinValue = DIRECT_PIN_READ(_aPin_register, _aPin_bitmask);
	if (newPinValue == _aPinValue) {
		return;
	}
	_aPinValue = newPinValue;
	if (_aPinValue) {
		pinChangeHandler(AR);
	} else {
		pinChangeHandler(AF);
	}
}

void NewEncoder::bPinChange() {
	uint8_t newPinValue = DIRECT_PIN_READ(_bPin_register, _bPin_bitmask);
	if (newPinValue == _bPinValue) {
		return;
	}
	_bPinValue = newPinValue;
	if (_bPinValue) {
		pinChangeHandler(BR);
	} else {
		pinChangeHandler(BF);
	}
}

void NewEncoder::pinChangeHandler(uint8_t index) {
	uint8_t newState;

#ifdef DEBUG_MODE
	if (fillLevel < fifoSize) {
		fillLevel++;
		fifo[writePointer++] = index;
		writePointer &= fifoMask;
	} else {
		overflow = true;
	}
#endif

	newState = NewEncoder::_transistionTable[_currentState][index];
	_currentState = newState & _stateMask;
	if ((newState & _deltaMask) == _incrementDelta) {
		if (_currentValue < _maxValue) {
			_currentValue++;
		}
	} else if ((newState & _deltaMask) == _decrementDelta) {
		if (_currentValue > _minValue) {
			_currentValue--;
		}
	}
}

void NewEncoder::aPinIsr() {
	for (uint8_t index = 0; index < _numEncoders; index++) {
		_encoderTable[index]->aPinChange();
	}
}

void NewEncoder::bPinIsr() {
	for (uint8_t index = 0; index < _numEncoders; index++) {
		_encoderTable[index]->bPinChange();
	}
}
