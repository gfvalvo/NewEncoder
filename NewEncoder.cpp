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

NewEncoder::NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue, int16_t initalValue, uint8_t type, DataProvider *dataProvider) {
	active = false;
	configure(aPin, bPin, minValue, maxValue, initalValue, type, dataProvider);
}

NewEncoder::NewEncoder() {
	active = false;
	configured = false;
}

NewEncoder::~NewEncoder() {
	end();
}

void NewEncoder::end() {
	if (!active) {
		return;
	}
	active = false;

	if (dataProvider) {
		dataProvider->end();
	}
}

void NewEncoder::configure(uint8_t aPin, uint8_t bPin, int16_t minValue,
		int16_t maxValue, int16_t initalValue, uint8_t type, DataProvider *dataProvider) {

	if (active) {
		end();
	}
	
	if (!dataProvider) {
		end();
		return;
	}

	this->dataProvider = dataProvider;
	dataProvider->configure(aPin, bPin, this);

	_minValue = minValue;
	_maxValue = maxValue;

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

	if (_minValue >= _maxValue) {
		return false;
	}

	if (!dataProvider->begin()) {
		return false;
	}

	currentStateVariable = (dataProvider->bPinValue() << 1) | dataProvider->aPinValue();
	if ((tablePtr == halfPulseTransitionTable) && (currentStateVariable == (DETENT_1 & 0b11))) {
		currentStateVariable = DETENT_1;
	}

	active = true;
	return true;
}

bool NewEncoder::getState(EncoderState &state) {
	bool localStateChanged = stateChanged;
	if (localStateChanged) {
		dataProvider->interruptOff();
		memcpy((void*) &localState, (void*) &liveState, sizeof(EncoderState));
		stateChanged = false;
		dataProvider->interruptOn();
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
	dataProvider->interruptOff();
	changed = stateChanged;
	stateChanged = false;
	memcpy((void*) &Oldstate, (void*) &liveState, sizeof(EncoderState));
	if (!changed) {
		Oldstate.currentClick = NoClick;
	}
	liveState.currentValue = val;
	liveState.currentClick = NoClick;
	memcpy((void*) &localState, (void*) &liveState, sizeof(EncoderState));
	dataProvider->interruptOn();
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
	dataProvider->interruptOff();
	stateChanged = false;
	liveState.currentValue = newCurrent;
	liveState.currentClick = NoClick;
	_minValue = newMin;
	_maxValue = newMax;
	memcpy((void*) &localState, (void*) &liveState, sizeof(EncoderState));
	dataProvider->interruptOn();
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
	dataProvider->interruptOff();  // 16-bit access not atomic on 8-bit processor
#endif
	liveState.currentValue = val;
#if defined(__AVR__)
	dataProvider->interruptOn();
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
	dataProvider->interruptOff();  // 16-bit access not atomic on 8-bit processor
#endif
	liveState.currentValue = val;
#if defined(__AVR__)
	dataProvider->interruptOn();
#endif
	return val;
}

int16_t NewEncoder::getValue() {
#if defined(__AVR__)
	int16_t val;
	dataProvider->interruptOff();  // 16-bit access not atomic on 8-bit processor
	val = liveState.currentValue;
	dataProvider->interruptOn();
	return val;
#else
	return liveState.currentValue;
#endif
}

NewEncoder::operator int16_t() const {
#if defined(__AVR__)
	int16_t val;
	dataProvider->interruptOff();  // 16-bit access not atomic on 8-bit processor
	val = liveState.currentValue;
	dataProvider->interruptOn();
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
	dataProvider->interruptOff()
	;
	localCurrentValue = liveState.currentValue;
	liveState.currentValue = val;
	dataProvider->interruptOn()
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
	dataProvider->interruptOff();  // 16-bit access not atomic on 8-bit processor
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
	dataProvider->interruptOn();
#endif
	return success;
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

void ESP_ISR NewEncoder::checkPinChange(uint8_t index) { 
	pinChangeHandler(index); 
};