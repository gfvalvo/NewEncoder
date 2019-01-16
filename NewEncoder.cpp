/*
 * NewEncoder.cpp
 */

#include "NewEncoder.h"

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

isrInfo NewEncoder::_isrTable[CORE_NUM_INTERRUPT];

NewEncoder::NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue, int16_t initalValue) {
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
	_currentValue = initalValue;
	active = false;
	configured = true;
}

NewEncoder::NewEncoder() {
	active = false;
	configured = false;
	_aPin_register = nullptr;
	_bPin_register = nullptr;
}

NewEncoder::~NewEncoder() {
	end();
}

void NewEncoder::end() {
	if (!active) {
		return;
	}
	active = false;
	detachInterrupt(_interruptA);
	detachInterrupt(_interruptB);
}

void NewEncoder::configure(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue, int16_t initalValue) {
	if (active) {
		end();
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
	_currentValue = initalValue;
	configured = true;
}

bool NewEncoder::begin() {
	if (active) {
		return false;
	}
	if (!configured) {
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
	if (_minValue >= _maxValue) {
		return false;
	}

	pinMode(_aPin, INPUT_PULLUP);
	pinMode(_bPin, INPUT_PULLUP);
	delay(2);  // Seems to help ensure first reading after pinMode is correct
	_aPinValue = DIRECT_PIN_READ(_aPin_register, _aPin_bitmask);
	_bPinValue = DIRECT_PIN_READ(_bPin_register, _bPin_bitmask);
	_currentState = (_bPinValue << 1) | _aPinValue;

	//_currentValue = (_minValue + _maxValue) / 2;
	if (_currentValue > _maxValue) {
		_currentValue = _maxValue;
	} else if (_currentValue < _minValue) {
		_currentValue = _minValue;
	}

	active = true;

	_isrTable[_interruptA].objectPtr = this;
	_isrTable[_interruptA].functPtr = &NewEncoder::aPinChange;
	if (!attachEncoderInterrupt(_interruptA)) {
		return false;
	}

	_isrTable[_interruptB].objectPtr = this;
	_isrTable[_interruptB].functPtr = &NewEncoder::bPinChange;
	if (!attachEncoderInterrupt(_interruptB)) {
		detachInterrupt(_interruptA);
		return false;
	}
	return true;
}

bool NewEncoder::enabled() {
	return active;
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

bool NewEncoder::upClick() {
	if (clickUp) {
		clickUp = false;
		return true;
	} else {
		return false;
	}
}

bool NewEncoder::downClick() {
	if (clickDown) {
		clickDown = false;
		return true;
	} else {
		return false;
	}
}

void NewEncoder::aPinChange() {
	uint8_t newPinValue = DIRECT_PIN_READ(_aPin_register, _aPin_bitmask);
	if (newPinValue == _aPinValue) {
		return;
	}
	_aPinValue = newPinValue;
	if (_aPinValue) {
		pinChangeHandler(aPinRising);
	} else {
		pinChangeHandler(aPinFalling);
	}
}

void NewEncoder::bPinChange() {
	uint8_t newPinValue = DIRECT_PIN_READ(_bPin_register, _bPin_bitmask);
	if (newPinValue == _bPinValue) {
		return;
	}
	_bPinValue = newPinValue;
	if (_bPinValue) {
		pinChangeHandler(bPinRising);
	} else {
		pinChangeHandler(bPinFalling);
	}
}

void NewEncoder::pinChangeHandler(uint8_t index) {
	uint8_t newState;

	newState = NewEncoder::_transistionTable[_currentState][index];
	_currentState = newState & _stateMask;

	if ((newState & _deltaMask) == _incrementDelta) {
		clickUp = true;
		clickDown = false;
		if (_currentValue < _maxValue) {
			_currentValue++;
		}
	} else if ((newState & _deltaMask) == _decrementDelta) {
		clickUp = false;
		clickDown = true;
		if (_currentValue > _minValue) {
			_currentValue--;
		}
	}
}

bool NewEncoder::attachEncoderInterrupt(uint8_t interruptNumber) {
	switch (interruptNumber) {
#if CORE_NUM_INTERRUPT > 0
		case 0:
			attachInterrupt(0, isr00, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 1
		case 1:
			attachInterrupt(1, isr01, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 2
		case 2:
			attachInterrupt(2, isr02, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 3
		case 3:
			attachInterrupt(3, isr03, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 4
		case 4:
			attachInterrupt(4, isr04, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 5
		case 5:
			attachInterrupt(5, isr05, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 6
		case 6:
			attachInterrupt(6, isr06, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 7
		case 7:
			attachInterrupt(7, isr07, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 8
		case 8:
			attachInterrupt(8, isr08, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 9
		case 9:
			attachInterrupt(9, isr09, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 10
		case 10:
			attachInterrupt(10, isr10, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 11
		case 11:
			attachInterrupt(11, isr11, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 12
		case 12:
			attachInterrupt(12, isr12, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 13
		case 13:
			attachInterrupt(13, isr13, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 14
		case 14:
			attachInterrupt(14, isr14, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 15
		case 15:
			attachInterrupt(15, isr15, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 16
		case 16:
			attachInterrupt(16, isr16, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 17
		case 17:
			attachInterrupt(17, isr17, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 18
		case 18:
			attachInterrupt(18, isr18, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 19
		case 19:
			attachInterrupt(19, isr19, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 20
		case 20:
			attachInterrupt(20, isr20, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 21
		case 21:
			attachInterrupt(21, isr21, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 22
		case 22:
			attachInterrupt(22, isr22, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 23
		case 23:
			attachInterrupt(23, isr23, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 24
		case 24:
			attachInterrupt(24, isr24, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 25
		case 25:
			attachInterrupt(25, isr25, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 26
		case 26:
			attachInterrupt(26, isr26, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 27
		case 27:
			attachInterrupt(27, isr27, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 28
		case 28:
			attachInterrupt(28, isr28, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 29
		case 29:
			attachInterrupt(29, isr29, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 30
		case 30:
			attachInterrupt(30, isr30, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 31
		case 31:
			attachInterrupt(31, isr31, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 32
		case 32:
			attachInterrupt(32, isr32, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 33
		case 33:
			attachInterrupt(33, isr33, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 34
			case 34:
			attachInterrupt(34, isr34, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 35
			case 35:
			attachInterrupt(35, isr35, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 36
			case 36:
			attachInterrupt(36, isr36, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 37
			case 37:
			attachInterrupt(37, isr37, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 38
			case 38:
			attachInterrupt(38, isr38, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 39
			case 39:
			attachInterrupt(39, isr39, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 40
			case 40:
			attachInterrupt(40, isr40, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 41
			case 41:
			attachInterrupt(41, isr41, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 42
			case 42:
			attachInterrupt(42, isr42, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 43
			case 43:
			attachInterrupt(43, isr43, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 44
			case 44:
			attachInterrupt(44, isr44, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 45
			case 45:
			attachInterrupt(45, isr45, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 46
			case 46:
			attachInterrupt(46, isr46, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 47
			case 47:
			attachInterrupt(47, isr47, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 48
			case 48:
			attachInterrupt(48, isr48, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 49
			case 49:
			attachInterrupt(49, isr49, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 50
			case 50:
			attachInterrupt(50, isr50, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 51
			case 51:
			attachInterrupt(51, isr51, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 52
			case 52:
			attachInterrupt(52, isr52, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 53
			case 53:
			attachInterrupt(53, isr53, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 54
			case 54:
			attachInterrupt(54, isr54, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 55
			case 55:
			attachInterrupt(55, isr55, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 56
			case 56:
			attachInterrupt(56, isr56, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 57
			case 57:
			attachInterrupt(57, isr57, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 58
			case 58:
			attachInterrupt(58, isr58, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 59
			case 59:
			attachInterrupt(59, isr59, CHANGE);
			break;
#endif

		default:
			return false;
	}
	return true;
}
