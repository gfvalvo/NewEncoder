/*
 * NewEncoder.cpp
 */

#include "NewEncoder.h"

#define A_PIN_FALLING 0b00
#define A_PIN_RISING 0b01
#define B_PIN_FALLING 0b10
#define B_PIN_FALLING 0b10
#define B_PIN_RISING  0b11

// Define states and transition table for "one pulse per detent" type encoder
#define START_STATE 0b011
#define CW_STATE_1 0b010
#define CW_STATE_2 0b000
#define CW_STATE_3 0b001
#define CCW_STATE_1 0b101
#define CCW_STATE_2 0b100
#define CCW_STATE_3 0b110

const encoderStateTransition NewEncoder::fullPulseTransitionTable[] = {
		{ CW_STATE_2, CW_STATE_3, CW_STATE_2, CW_STATE_1 }, // cwState2 = 0b000
		{ CW_STATE_2, CW_STATE_3, CW_STATE_3, START_STATE | INCREMENT_DELTA }, // cwState3 = 0b001
		{ CW_STATE_1, START_STATE, CW_STATE_2, CW_STATE_1 },  // cwState1 = 0b010
		{ CW_STATE_1, START_STATE, CCW_STATE_1, START_STATE }, // startState = 0b011
		{ CCW_STATE_2, CCW_STATE_1, CCW_STATE_2, CCW_STATE_3 }, // ccwState2 = 0b100
		{ CCW_STATE_2, CCW_STATE_1, CCW_STATE_1, START_STATE }, // ccwState1 = 0b101
		{ CCW_STATE_3, START_STATE | DECREMENT_DELTA, CCW_STATE_2, CCW_STATE_3 }, // ccwState3 = 0b110
		{ START_STATE, START_STATE, START_STATE, START_STATE } // 0b111 illegal state should never be in it
};

// Define states and transition table for "one pulse per two detents" type encoder
#define DETENT_0 0b000
#define DETENT_1 0b111
#define DEBOUNCE_0 0b010
#define DEBOUNCE_1 0b001
#define DEBOUNCE_2 0b101
#define DEBOUNCE_3 0b110

const encoderStateTransition NewEncoder::halfPulseTransitionTable[] = {
		{ DETENT_0, DEBOUNCE_1, DETENT_0, DEBOUNCE_0 },  // DETENT_0 0b000
		{ DETENT_0, DEBOUNCE_1, DEBOUNCE_1, DETENT_1 | INCREMENT_DELTA }, // DEBOUNCE_1 0b001
		{ DEBOUNCE_0, DETENT_1 | DECREMENT_DELTA, DETENT_0, DEBOUNCE_0 },  // DEBOUNCE_0 0b010
		{ DETENT_1, DETENT_1, DETENT_1, DETENT_1 },  // 0b011 - illegal state should never be in it
		{ DETENT_0, DETENT_0, DETENT_0, DETENT_0 },  // 0b100 - illegal state should never be in it
		{ DETENT_0 | DECREMENT_DELTA, DEBOUNCE_2, DEBOUNCE_2, DETENT_1 }, // DEBOUNCE_2 0b101
		{ DEBOUNCE_3, DETENT_1, DETENT_0 | INCREMENT_DELTA, DEBOUNCE_3 },  // DEBOUNCE_3 0b110
		{ DEBOUNCE_3, DETENT_1, DEBOUNCE_2, DETENT_1 }  // DETENT_1 0b111
};

isrInfo NewEncoder::_isrTable[CORE_NUM_INTERRUPT];

NewEncoder::NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue, int16_t initalValue, uint8_t type) {
	active = false;
	configure(aPin, bPin, minValue, maxValue, initalValue, type);
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

	int16_t _interruptA = digitalPinToInterrupt(_aPin);
	int16_t _interruptB = digitalPinToInterrupt(_bPin);
	detachInterrupt(_interruptA);
	detachInterrupt(_interruptB);
}

void NewEncoder::configure(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue, int16_t initalValue, uint8_t type) {

	if (active) {
		end();
	}
	_aPin = aPin;
	_bPin = bPin;
	_minValue = minValue;
	_maxValue = maxValue;
	_aPin_register = PIN_TO_BASEREG(aPin);
	_bPin_register = PIN_TO_BASEREG(bPin);
	_aPin_bitmask = PIN_TO_BITMASK(aPin);
	_bPin_bitmask = PIN_TO_BITMASK(bPin);

	if (initalValue > _maxValue) {
		initalValue = _maxValue;
	} else if (initalValue < _minValue) {
		initalValue = _minValue;
	}

	liveState.currentValue = initalValue;
	liveState.currentClick = NoClick;
	memcpy((void *) &localState, (void *) &liveState, sizeof(EncoderState));
	stateChanged = false;

	if (type == HALF_PULSE) {
		tablePtr = halfPulseTransitionTable;
	} else {
		tablePtr = fullPulseTransitionTable;
	}
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

	int16_t _interruptA = digitalPinToInterrupt(_aPin);
	int16_t _interruptB = digitalPinToInterrupt(_bPin);

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
	currentStateVariable = (_bPinValue << 1) | _aPinValue;
	if ((tablePtr == halfPulseTransitionTable) && (currentStateVariable == (DETENT_1 & 0b11))) {
		currentStateVariable = DETENT_1;
	}

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
	active = true;
	return true;
}

bool NewEncoder::getState(EncoderState &state) {
	bool localStateChanged = stateChanged;
	if (localStateChanged) {
		noInterrupts();
		memcpy((void *) &localState, (void *) &liveState, sizeof(EncoderState));
		stateChanged = false;
		interrupts();
	} else {
		localState.currentClick = NoClick;
	}
	memcpy((void *) &state, (void *) &localState, sizeof(EncoderState));
	return localStateChanged;
}

bool NewEncoder::getAndSet(int16_t val, EncoderState &Oldstate, EncoderState &Newstate) {
	bool changed;
	if (val < _minValue) {
		val = _minValue;
	} else if (val > _maxValue) {
		val = _maxValue;
	}
	noInterrupts();
	changed = stateChanged;
	stateChanged = false;
	memcpy((void *) &Oldstate, (void *) &liveState, sizeof(EncoderState));
	if (!changed) {
		Oldstate.currentClick = NoClick;
	}
	liveState.currentValue = val;
	liveState.currentClick = NoClick;
	memcpy((void *) &localState, (void *) &liveState, sizeof(EncoderState));
	interrupts();
	memcpy((void *) &Newstate, (void *) &localState, sizeof(EncoderState));
	return changed;
}

bool NewEncoder::newSettings(int16_t newMin, int16_t newMax, int16_t newCurrent, EncoderState &state) {
	if (newMax <= newMin) {
		return false;
	}
	if (newCurrent < newMin) {
		newCurrent = newMin;
	}
	if (newCurrent > newMax) {
		newCurrent = newMax;
	}
	noInterrupts();
	stateChanged = false;
	liveState.currentValue = newCurrent;
	liveState.currentClick = NoClick;
	memcpy((void *) &localState, (void *) &liveState, sizeof(EncoderState));
	interrupts();
	memcpy((void *) &state, (void *) &localState, sizeof(EncoderState));
	return true;
}

bool NewEncoder::enabled() {
	return active;
}

void NewEncoder::attachCallback(EncoderCallBack cback, void *uPtr) {
	callBackPtr = cback;
	userPointer = uPtr;
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
	liveState.currentValue = val;
#if defined(__AVR__)
	interrupts();
#endif
	return val;
}

int16_t NewEncoder::operator =(int16_t val) {
	if (val < _minValue) {
		val = _minValue;
	} else if (val > _maxValue) {
		val = _maxValue;
	}
#if defined(__AVR__)
	noInterrupts();  // 16-bit access not atomic on 8-bit processor
#endif
	liveState.currentValue = val;
#if defined(__AVR__)
	interrupts();
#endif
	return val;
}

int16_t NewEncoder::getValue() {
#if defined(__AVR__)
	int16_t val;
	noInterrupts();  // 16-bit access not atomic on 8-bit processor
	val = liveState.currentValue;
	interrupts();
	return val;
#else
	return liveState.currentValue;
#endif
}

NewEncoder::operator int16_t() const {
#if defined(__AVR__)
	int16_t val;
	noInterrupts();  // 16-bit access not atomic on 8-bit processor
	val = liveState.currentValue;
	interrupts();
	return val;
#else
	return liveState.currentValue;
#endif
}

int16_t NewEncoder::getAndSet(int16_t val) {
	int16_t localCurrentValue;
	if (val < _minValue) {
		val = _minValue;
	} else if (val > _maxValue) {
		val = _maxValue;
	}
	noInterrupts()
	;
	localCurrentValue = liveState.currentValue;
	liveState.currentValue = val;
	interrupts()
	;
	return localCurrentValue;
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

bool NewEncoder::newSettings(int16_t newMin, int16_t newMax, int16_t newCurrent) {
	bool success = false;
#if defined(__AVR__)
	noInterrupts();  // 16-bit access not atomic on 8-bit processor
#endif
	if (active) {
		if (newMax > newMin) {
			if (newCurrent < newMin) {
				newCurrent = newMin;
			} else if (newCurrent > newMax) {
				newCurrent = newMax;
			}
			liveState.currentValue = newCurrent;
			_minValue = newMin;
			_maxValue = newMax;
			success = true;
		}
	}
#if defined(__AVR__)
	interrupts();
#endif
	return success;
}

void ESP_ISR NewEncoder::aPinChange() {
	uint8_t newPinValue = DIRECT_PIN_READ(_aPin_register, _aPin_bitmask);
	if (newPinValue == _aPinValue) {
		return;
	}
	_aPinValue = newPinValue;
	pinChangeHandler(0b00 | _aPinValue);  // Falling aPin == 0b00, Rising aPin = 0b01;
}

void ESP_ISR NewEncoder::bPinChange() {
	uint8_t newPinValue = DIRECT_PIN_READ(_bPin_register, _bPin_bitmask);
	if (newPinValue == _bPinValue) {
		return;
	}
	_bPinValue = newPinValue;
	pinChangeHandler(0b10 | _bPinValue);  // Falling bPin == 0b10, Rising bPin = 0b11;
}

void ESP_ISR NewEncoder::pinChangeHandler(uint8_t index) {
	uint8_t newStateVariable;

	newStateVariable = NewEncoder::tablePtr[currentStateVariable][index];
	currentStateVariable = newStateVariable & STATE_MASK;
	if ((newStateVariable & DELTA_MASK) != 0) {
		if ((newStateVariable & DELTA_MASK) == INCREMENT_DELTA) {
			clickUp = true;
			clickDown = false;
		} else {
			clickUp = false;
			clickDown = true;
		}
		updateValue(newStateVariable);
		if (callBackPtr != nullptr) {
			callBackPtr(this, &liveState, userPointer);
		}
	}
}

void ESP_ISR NewEncoder::updateValue(uint8_t updatedStateVariable) {
	if ((updatedStateVariable & DELTA_MASK) == INCREMENT_DELTA) {
		liveState.currentClick = UpClick;
		if (liveState.currentValue < _maxValue) {
			liveState.currentValue++;
		}
	} else if ((updatedStateVariable & DELTA_MASK) == DECREMENT_DELTA) {
		liveState.currentClick = DownClick;
		if (liveState.currentValue > _minValue) {
			liveState.currentValue--;
		}
	}
	stateChanged = true;
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
#if CORE_NUM_INTERRUPT > 60
			case 60:
			attachInterrupt(60, isr60, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 61
			case 61:
			attachInterrupt(61, isr61, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 62
			case 62:
			attachInterrupt(62, isr62, CHANGE);
			break;
#endif
#if CORE_NUM_INTERRUPT > 63
			case 63:
			attachInterrupt(63, isr63, CHANGE);
			break;
#endif
		default:
			return false;
	}
	return true;
}

#if CORE_NUM_INTERRUPT > 0
void ESP_ISR NewEncoder::isr00(void) {
	CALL_MEMBER_FN(_isrTable[0].objectPtr, _isrTable[0].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 1
void ESP_ISR NewEncoder::isr01(void) {
	CALL_MEMBER_FN(_isrTable[1].objectPtr, _isrTable[1].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 2
void ESP_ISR NewEncoder::isr02(void) {
	CALL_MEMBER_FN(_isrTable[2].objectPtr, _isrTable[2].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 3
void ESP_ISR NewEncoder::isr03(void) {
	CALL_MEMBER_FN(_isrTable[3].objectPtr, _isrTable[3].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 4
void ESP_ISR NewEncoder::isr04(void) {
	CALL_MEMBER_FN(_isrTable[4].objectPtr, _isrTable[4].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 5
void ESP_ISR NewEncoder::isr05(void) {
	CALL_MEMBER_FN(_isrTable[5].objectPtr, _isrTable[5].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 6
void ESP_ISR NewEncoder::isr06(void) {
	CALL_MEMBER_FN(_isrTable[6].objectPtr, _isrTable[6].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 7
void ESP_ISR NewEncoder::isr07(void) {
	CALL_MEMBER_FN(_isrTable[7].objectPtr, _isrTable[7].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 8
void ESP_ISR NewEncoder::isr08(void) {
	CALL_MEMBER_FN(_isrTable[8].objectPtr, _isrTable[8].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 9
void ESP_ISR NewEncoder::isr09(void) {
	CALL_MEMBER_FN(_isrTable[9].objectPtr, _isrTable[9].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 10
void ESP_ISR NewEncoder::isr10(void) {
	CALL_MEMBER_FN(_isrTable[10].objectPtr, _isrTable[10].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 11
void ESP_ISR NewEncoder::isr11(void) {
	CALL_MEMBER_FN(_isrTable[11].objectPtr, _isrTable[11].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 12
void ESP_ISR NewEncoder::isr12(void) {
	CALL_MEMBER_FN(_isrTable[12].objectPtr, _isrTable[12].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 13
void ESP_ISR NewEncoder::isr13(void) {
	CALL_MEMBER_FN(_isrTable[13].objectPtr, _isrTable[13].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 14
void ESP_ISR NewEncoder::isr14(void) {
	CALL_MEMBER_FN(_isrTable[14].objectPtr, _isrTable[14].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 15
void ESP_ISR NewEncoder::isr15(void) {
	CALL_MEMBER_FN(_isrTable[15].objectPtr, _isrTable[15].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 16
void ESP_ISR NewEncoder::isr16(void) {
	CALL_MEMBER_FN(_isrTable[16].objectPtr, _isrTable[16].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 17
void ESP_ISR NewEncoder::isr17(void) {
	CALL_MEMBER_FN(_isrTable[17].objectPtr, _isrTable[17].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 18
void ESP_ISR NewEncoder::isr18(void) {
	CALL_MEMBER_FN(_isrTable[18].objectPtr, _isrTable[18].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 19
void ESP_ISR NewEncoder::isr19(void) {
	CALL_MEMBER_FN(_isrTable[19].objectPtr, _isrTable[19].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 20
void ESP_ISR NewEncoder::isr20(void) {
	CALL_MEMBER_FN(_isrTable[20].objectPtr, _isrTable[20].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 21
void ESP_ISR NewEncoder::isr21(void) {
	CALL_MEMBER_FN(_isrTable[21].objectPtr, _isrTable[21].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 22
void ESP_ISR NewEncoder::isr22(void) {
	CALL_MEMBER_FN(_isrTable[22].objectPtr, _isrTable[22].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 23
void ESP_ISR NewEncoder::isr23(void) {
	CALL_MEMBER_FN(_isrTable[23].objectPtr, _isrTable[23].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 24
void ESP_ISR NewEncoder::isr24(void) {
	CALL_MEMBER_FN(_isrTable[24].objectPtr, _isrTable[24].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 25
void ESP_ISR NewEncoder::isr25(void) {
	CALL_MEMBER_FN(_isrTable[25].objectPtr, _isrTable[25].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 26
void ESP_ISR NewEncoder::isr26(void) {
	CALL_MEMBER_FN(_isrTable[26].objectPtr, _isrTable[26].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 27
void ESP_ISR NewEncoder::isr27(void) {
	CALL_MEMBER_FN(_isrTable[27].objectPtr, _isrTable[27].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 28
void ESP_ISR NewEncoder::isr28(void) {
	CALL_MEMBER_FN(_isrTable[28].objectPtr, _isrTable[28].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 29
void ESP_ISR NewEncoder::isr29(void) {
	CALL_MEMBER_FN(_isrTable[29].objectPtr, _isrTable[29].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 30
void ESP_ISR NewEncoder::isr30(void) {
	CALL_MEMBER_FN(_isrTable[30].objectPtr, _isrTable[30].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 31
void ESP_ISR NewEncoder::isr31(void) {
	CALL_MEMBER_FN(_isrTable[31].objectPtr, _isrTable[31].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 32
void ESP_ISR NewEncoder::isr32(void) {
	CALL_MEMBER_FN(_isrTable[32].objectPtr, _isrTable[32].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 33
void ESP_ISR NewEncoder::isr33(void) {
	CALL_MEMBER_FN(_isrTable[33].objectPtr, _isrTable[33].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 34
void ESP_ISR NewEncoder::isr34(void) {
	CALL_MEMBER_FN(_isrTable[34].objectPtr, _isrTable[34].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 35
void ESP_ISR NewEncoder::isr35(void) {
	CALL_MEMBER_FN(_isrTable[35].objectPtr, _isrTable[35].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 36
void ESP_ISR NewEncoder::isr36(void) {
	CALL_MEMBER_FN(_isrTable[36].objectPtr, _isrTable[36].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 37
void ESP_ISR NewEncoder::isr37(void) {
	CALL_MEMBER_FN(_isrTable[37].objectPtr, _isrTable[37].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 38
void ESP_ISR NewEncoder::isr38(void) {
	CALL_MEMBER_FN(_isrTable[38].objectPtr, _isrTable[38].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 39
void ESP_ISR NewEncoder::isr39(void) {
	CALL_MEMBER_FN(_isrTable[39].objectPtr, _isrTable[39].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 40
void ESP_ISR NewEncoder::isr40(void) {
	CALL_MEMBER_FN(_isrTable[40].objectPtr, _isrTable[40].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 41
void ESP_ISR NewEncoder::isr41(void) {
	CALL_MEMBER_FN(_isrTable[41].objectPtr, _isrTable[41].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 42
void ESP_ISR NewEncoder::isr42(void) {
	CALL_MEMBER_FN(_isrTable[42].objectPtr, _isrTable[42].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 43
void ESP_ISR NewEncoder::isr43(void) {
	CALL_MEMBER_FN(_isrTable[43].objectPtr, _isrTable[43].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 44
void ESP_ISR NewEncoder::isr44(void) {
	CALL_MEMBER_FN(_isrTable[44].objectPtr, _isrTable[44].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 45
void ESP_ISR NewEncoder::isr45(void) {
	CALL_MEMBER_FN(_isrTable[45].objectPtr, _isrTable[45].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 46
void ESP_ISR NewEncoder::isr46(void) {
	CALL_MEMBER_FN(_isrTable[46].objectPtr, _isrTable[46].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 47
void ESP_ISR NewEncoder::isr47(void) {
	CALL_MEMBER_FN(_isrTable[47].objectPtr, _isrTable[47].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 48
void ESP_ISR NewEncoder::isr48(void) {
	CALL_MEMBER_FN(_isrTable[48].objectPtr, _isrTable[48].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 49
void ESP_ISR NewEncoder::isr49(void) {
	CALL_MEMBER_FN(_isrTable[49].objectPtr, _isrTable[49].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 50
void ESP_ISR NewEncoder::isr50(void) {
	CALL_MEMBER_FN(_isrTable[50].objectPtr, _isrTable[50].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 51
void ESP_ISR NewEncoder::isr51(void) {
	CALL_MEMBER_FN(_isrTable[51].objectPtr, _isrTable[51].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 52
void ESP_ISR NewEncoder::isr52(void) {
	CALL_MEMBER_FN(_isrTable[52].objectPtr, _isrTable[52].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 53
void ESP_ISR NewEncoder::isr53(void) {
	CALL_MEMBER_FN(_isrTable[53].objectPtr, _isrTable[53].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 54
void ESP_ISR NewEncoder::isr54(void) {
	CALL_MEMBER_FN(_isrTable[54].objectPtr, _isrTable[54].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 55
void ESP_ISR NewEncoder::isr55(void) {
	CALL_MEMBER_FN(_isrTable[55].objectPtr, _isrTable[55].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 56
void ESP_ISR NewEncoder::isr56(void) {
	CALL_MEMBER_FN(_isrTable[56].objectPtr, _isrTable[56].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 57
void ESP_ISR NewEncoder::isr57(void) {
	CALL_MEMBER_FN(_isrTable[57].objectPtr, _isrTable[57].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 58
void ESP_ISR NewEncoder::isr58(void) {
	CALL_MEMBER_FN(_isrTable[58].objectPtr, _isrTable[58].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 59
void ESP_ISR NewEncoder::isr59(void) {
	CALL_MEMBER_FN(_isrTable[59].objectPtr, _isrTable[59].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 60
void ESP_ISR NewEncoder::isr60(void) {
	CALL_MEMBER_FN(_isrTable[60].objectPtr, _isrTable[60].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 61
void ESP_ISR NewEncoder::isr61(void) {
	CALL_MEMBER_FN(_isrTable[61].objectPtr, _isrTable[61].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 62
void ESP_ISR NewEncoder::isr62(void) {
	CALL_MEMBER_FN(_isrTable[62].objectPtr, _isrTable[62].functPtr);
}
#endif
#if CORE_NUM_INTERRUPT > 63
void ESP_ISR NewEncoder::isr63(void) {
	CALL_MEMBER_FN(_isrTable[63].objectPtr, _isrTable[63].functPtr);
}
#endif

