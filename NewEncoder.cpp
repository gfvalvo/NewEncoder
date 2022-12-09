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

const NewEncoder::encoderStateTransition NewEncoder::fullPulseTransitionTable[] = {
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

const NewEncoder::encoderStateTransition NewEncoder::halfPulseTransitionTable[] = {
		{ DETENT_0, DEBOUNCE_1, DETENT_0, DEBOUNCE_0 },  // DETENT_0 0b000
		{ DETENT_0, DEBOUNCE_1, DEBOUNCE_1, DETENT_1 | INCREMENT_DELTA }, // DEBOUNCE_1 0b001
		{ DEBOUNCE_0, DETENT_1 | DECREMENT_DELTA, DETENT_0, DEBOUNCE_0 },  // DEBOUNCE_0 0b010
		{ DETENT_1, DETENT_1, DETENT_1, DETENT_1 },  // 0b011 - illegal state should never be in it
		{ DETENT_0, DETENT_0, DETENT_0, DETENT_0 },  // 0b100 - illegal state should never be in it
		{ DETENT_0 | DECREMENT_DELTA, DEBOUNCE_2, DEBOUNCE_2, DETENT_1 }, // DEBOUNCE_2 0b101
		{ DEBOUNCE_3, DETENT_1, DETENT_0 | INCREMENT_DELTA, DEBOUNCE_3 },  // DEBOUNCE_3 0b110
		{ DEBOUNCE_3, DETENT_1, DEBOUNCE_2, DETENT_1 }  // DETENT_1 0b111
};

#ifndef USE_FUNCTIONAL_ISR
NewEncoder::isrInfo NewEncoder::_isrTable[CORE_NUM_INTERRUPT];
#endif

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
	memcpy((void*) &localState, (void*) &liveState, sizeof(EncoderState));
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

	using InterruptNumberType = decltype(NOT_AN_INTERRUPT);

	InterruptNumberType _interruptA = static_cast<InterruptNumberType>(digitalPinToInterrupt(_aPin));
	InterruptNumberType _interruptB = static_cast<InterruptNumberType>(digitalPinToInterrupt(_bPin));

	if (_interruptA == _interruptB) {
		return false;
	}
	if (_interruptA == NOT_AN_INTERRUPT) {
		return false;
	}
	if (_interruptB == NOT_AN_INTERRUPT) {
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

#ifndef USE_FUNCTIONAL_ISR
	_isrTable[_interruptA].objectPtr = this;
	_isrTable[_interruptA].functPtr = &NewEncoder::aPinChange;
	auto isrA = getIsr(_interruptA);
	if (isrA == nullptr) {
		return false;
	}

	_isrTable[_interruptB].objectPtr = this;
	_isrTable[_interruptB].functPtr = &NewEncoder::bPinChange;
	auto isrB = getIsr(_interruptB);
	if (isrB == nullptr) {
		return false;
	}

	attachInterrupt(_interruptA, isrA, CHANGE);
	attachInterrupt(_interruptB, isrB, CHANGE);

#else
	auto aPinIsr = [this] {
		this->aPinChange();
	};
	attachInterrupt(_interruptA, aPinIsr, CHANGE);

	auto bPinIsr = [this] {
		this->bPinChange();
	};
	attachInterrupt(_interruptB, bPinIsr, CHANGE);

#endif
	active = true;
	return true;
}

bool NewEncoder::getState(EncoderState &state) {
	bool localStateChanged = stateChanged;
	if (localStateChanged) {
		noInterrupts();
		memcpy((void*) &localState, (void*) &liveState, sizeof(EncoderState));
		stateChanged = false;
		interrupts();
	} else {
		localState.currentClick = NoClick;
	}
	memcpy((void*) &state, (void*) &localState, sizeof(EncoderState));
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
	memcpy((void*) &Oldstate, (void*) &liveState, sizeof(EncoderState));
	if (!changed) {
		Oldstate.currentClick = NoClick;
	}
	liveState.currentValue = val;
	liveState.currentClick = NoClick;
	memcpy((void*) &localState, (void*) &liveState, sizeof(EncoderState));
	interrupts();
	memcpy((void*) &Newstate, (void*) &localState, sizeof(EncoderState));
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
	_minValue = newMin;
	_maxValue = newMax;
	memcpy((void*) &localState, (void*) &liveState, sizeof(EncoderState));
	interrupts();
	memcpy((void*) &state, (void*) &localState, sizeof(EncoderState));
	return true;
}

bool NewEncoder::enabled() const {
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
