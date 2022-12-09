/*
 * NewEncoder.h
 */
#ifndef NEWENCODER_H_
#define NEWENCODER_H_

#undef USE_FUNCTIONAL_ISR
#undef ESP_ISR

#if defined(ESP8266) || defined(ESP32)
#define ESP_ISR IRAM_ATTR
#define USE_FUNCTIONAL_ISR
#include "FunctionalInterrupt.h"
#else
#define ESP_ISR
#endif

#ifdef ARDUINO_ARCH_STM32
#define USE_FUNCTIONAL_ISR
#endif

#define STATE_MASK 0b00000111
#define DELTA_MASK 0b00011000
#define INCREMENT_DELTA 0b00001000
#define DECREMENT_DELTA 0b00010000

#include <Arduino.h>
#include "utility/interrupt_pins.h"
#include "utility/direct_pin_read.h"

#define FULL_PULSE 0
#define HALF_PULSE 1

class NewEncoder {

public:
	enum EncoderClick {
		NoClick, DownClick, UpClick
	};

	struct EncoderState {
		int16_t currentValue = 0;
		EncoderClick currentClick = NoClick;
	};

private:
	using EncoderCallBack = void(*)(NewEncoder*, const volatile EncoderState*, void*);
	using encoderStateTransition = uint8_t[4];

public:
	NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE);
	NewEncoder();
	virtual ~NewEncoder();
	virtual bool begin();
	virtual void configure(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE);
	virtual void end();
	bool enabled() const;
	void attachCallback(EncoderCallBack cback, void *uPtr = nullptr);
	bool getState(EncoderState &state);
	bool getAndSet(int16_t val, EncoderState &Oldstate, EncoderState &Newstate);
	bool newSettings(int16_t newMin, int16_t newMax, int16_t newCurrent, EncoderState &state);

	NewEncoder(const NewEncoder&) = delete; // delete copy constructor. no copying allowed
	NewEncoder& operator=(const NewEncoder&) = delete; // delete operator=(). no assignment allowed

protected:
	// This function may be implemented in an inherited class to customize the increment/decrement and min/max behavior.
	// See the source code and CustomEncoder example
	// Caution - this function is called in interrupt context.
	virtual void updateValue(uint8_t updatedState);

	volatile int16_t _minValue = 0, _maxValue = 0;
	volatile EncoderState liveState;
	volatile bool stateChanged;
	EncoderState localState;
	bool configured = false;

private:
	void pinChangeHandler(uint8_t index);
	void aPinChange();
	void bPinChange();
	bool active = false;

	uint8_t _aPin = 0, _bPin = 0;
	const encoderStateTransition *tablePtr = nullptr;
	volatile uint8_t _aPinValue, _bPinValue;
	volatile uint8_t currentStateVariable;
	volatile IO_REG_TYPE *_aPin_register;
	volatile IO_REG_TYPE *_bPin_register;
	volatile IO_REG_TYPE _aPin_bitmask;
	volatile IO_REG_TYPE _bPin_bitmask;
	volatile bool clickUp = false;
	volatile bool clickDown = false;

	static const encoderStateTransition fullPulseTransitionTable[];
	static const encoderStateTransition halfPulseTransitionTable[];

	EncoderCallBack callBackPtr = nullptr;
	void *userPointer = nullptr;

#ifndef USE_FUNCTIONAL_ISR
	using PinChangeFunction = void (NewEncoder::*)();
	using isrFunct = void (*)();

	struct isrInfo {
		NewEncoder *objectPtr;
		PinChangeFunction functPtr;
	};
	static isrInfo _isrTable[CORE_NUM_INTERRUPT];

	template<uint8_t NUM_INTERRUPTS = CORE_NUM_INTERRUPT>
	static isrFunct getIsr(uint8_t intNumber);
#endif

// -------- Deprecated Public Functions ---------
public:

	[[deprecated ("May be removed in future release. See README and library examples.")]]
	int16_t setValue(int16_t);

	[[deprecated ("May be removed in future release. See README and library examples.")]]
	int16_t getValue();

	[[deprecated ("May be removed in future release. See README and library examples.")]]
	int16_t operator=(int16_t val);

	[[deprecated ("May be removed in future release. See README and library examples.")]]
	int16_t getAndSet(int16_t val = 0);

	[[deprecated ("May be removed in future release. See README and library examples.")]]
	operator int16_t() const;

	[[deprecated ("May be removed in future release. See README and library examples.")]]
	bool upClick();

	[[deprecated ("May be removed in future release. See README and library examples.")]]
	bool downClick();

	[[deprecated ("May be removed in future release. See README and library examples.")]]
	bool newSettings(int16_t newMin, int16_t newMax, int16_t newCurrent);
};

#ifndef USE_FUNCTIONAL_ISR
template<uint8_t NUM_INTERRUPTS>
NewEncoder::isrFunct NewEncoder::getIsr(uint8_t intNumber) {
	if (intNumber == (NUM_INTERRUPTS - 1)) {
		return [] {
			((_isrTable[NUM_INTERRUPTS - 1].objectPtr)->*(_isrTable[NUM_INTERRUPTS - 1].functPtr))();
		};
	}
	return getIsr<NUM_INTERRUPTS - 1>(intNumber);
}

template<>
inline NewEncoder::isrFunct NewEncoder::getIsr<0>(uint8_t intNum) {
	(void) intNum;
	return nullptr;
}
#endif

#endif /* NEWENCODER_H_ */
