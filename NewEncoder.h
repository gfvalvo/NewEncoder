/*
 * NewEncoder.h
 */
#ifndef NEWENCODER_H_
#define NEWENCODER_H_

#if defined(ESP8266) || defined(ESP32)
#define ESP_ISR IRAM_ATTR
#else
#define ESP_ISR
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

class NewEncoder;
typedef void (NewEncoder::*PinChangeFunction)();

struct isrInfo {
	NewEncoder *objectPtr;
	PinChangeFunction functPtr;
};

typedef uint8_t encoderStateTransition[4];

#define CALL_MEMBER_FN(objectPtr, functPtr) ((objectPtr)->*(functPtr))()

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
	typedef void (*EncoderCallBack)(NewEncoder *, const volatile EncoderState *, void *);

public:
	NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE);
	NewEncoder();
	virtual ~NewEncoder();
	bool begin();
	virtual void configure(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE);
	void end();
	bool enabled();
	void attachCallback(EncoderCallBack cback, void *uPtr = nullptr);
	bool getState(EncoderState &state);
	bool getAndSet(int16_t val, EncoderState &Oldstate, EncoderState &Newstate);
	bool newSettings(int16_t newMin, int16_t newMax, int16_t newCurrent, EncoderState &state);

	NewEncoder(const NewEncoder &) = delete; // delete copy constructor. no copying allowed
	NewEncoder &operator=(const NewEncoder &) = delete; // delete operator=. no assignment allowed

protected:
    // This function may be implemented in an inherited class to customize the increment/decrement and min/max behavior.
    // See the source code and CustomEncoder example
    // Caution - this function is called in interrupt context.
	virtual void updateValue(uint8_t updatedState);

	volatile int16_t _minValue = 0, _maxValue = 0;
	volatile EncoderState liveState;
	volatile bool stateChanged;
	EncoderState localState;

private:
	void pinChangeHandler(uint8_t index);
	void aPinChange();
	void bPinChange();
	bool active = false;
	bool configured = false;
	uint8_t _aPin = 0, _bPin = 0;
	const encoderStateTransition *tablePtr = nullptr;
	volatile uint8_t _aPinValue, _bPinValue;
	volatile uint8_t currentStateVariable;
	volatile IO_REG_TYPE * _aPin_register;
	volatile IO_REG_TYPE * _bPin_register;
	volatile IO_REG_TYPE _aPin_bitmask;
	volatile IO_REG_TYPE _bPin_bitmask;
	volatile bool clickUp = false;
	volatile bool clickDown = false;

	static const encoderStateTransition fullPulseTransitionTable[];
	static const encoderStateTransition halfPulseTransitionTable[];

	static isrInfo _isrTable[CORE_NUM_INTERRUPT];

	static bool attachEncoderInterrupt(uint8_t interruptNumber);
	EncoderCallBack callBackPtr = nullptr;
	void *userPointer = nullptr;

#if CORE_NUM_INTERRUPT > 0
	static ESP_ISR void isr00(void);
	#endif
#if CORE_NUM_INTERRUPT > 1
	static ESP_ISR void isr01(void);
	#endif
#if CORE_NUM_INTERRUPT > 2
	static ESP_ISR void isr02(void);
	#endif
#if CORE_NUM_INTERRUPT > 3
	static ESP_ISR void isr03(void);
	#endif
#if CORE_NUM_INTERRUPT > 4
	static ESP_ISR void isr04(void);
	#endif
#if CORE_NUM_INTERRUPT > 5
	static ESP_ISR void isr05(void);
	#endif
#if CORE_NUM_INTERRUPT > 6
	static ESP_ISR void isr06(void);
	#endif
#if CORE_NUM_INTERRUPT > 7
	static ESP_ISR void isr07(void);
	#endif
#if CORE_NUM_INTERRUPT > 8
	static ESP_ISR void isr08(void);
	#endif
#if CORE_NUM_INTERRUPT > 9
	static ESP_ISR void isr09(void);
	#endif
#if CORE_NUM_INTERRUPT > 10
	static ESP_ISR void isr10(void);
	#endif
#if CORE_NUM_INTERRUPT > 11
	static ESP_ISR void isr11(void);
	#endif
#if CORE_NUM_INTERRUPT > 12
	static ESP_ISR void isr12(void);
	#endif
#if CORE_NUM_INTERRUPT > 13
	static ESP_ISR void isr13(void);
	#endif
#if CORE_NUM_INTERRUPT > 14
	static ESP_ISR void isr14(void);
	#endif
#if CORE_NUM_INTERRUPT > 15
	static ESP_ISR void isr15(void);
	#endif
#if CORE_NUM_INTERRUPT > 16
	static ESP_ISR void isr16(void);
	#endif
#if CORE_NUM_INTERRUPT > 17
	static ESP_ISR void isr17(void);
	#endif
#if CORE_NUM_INTERRUPT > 18
	static ESP_ISR void isr18(void);
	#endif
#if CORE_NUM_INTERRUPT > 19
	static ESP_ISR void isr19(void);
	#endif
#if CORE_NUM_INTERRUPT > 20
	static ESP_ISR void isr20(void);
	#endif
#if CORE_NUM_INTERRUPT > 21
	static ESP_ISR void isr21(void);
	#endif
#if CORE_NUM_INTERRUPT > 22
	static ESP_ISR void isr22(void);
	#endif
#if CORE_NUM_INTERRUPT > 23
	static ESP_ISR void isr23(void);
	#endif
#if CORE_NUM_INTERRUPT > 24
	static ESP_ISR void isr24(void);
	#endif
#if CORE_NUM_INTERRUPT > 25
	static ESP_ISR void isr25(void);
	#endif
#if CORE_NUM_INTERRUPT > 26
	static ESP_ISR void isr26(void);
	#endif
#if CORE_NUM_INTERRUPT > 27
	static ESP_ISR void isr27(void);
	#endif
#if CORE_NUM_INTERRUPT > 28
	static ESP_ISR void isr28(void);
	#endif
#if CORE_NUM_INTERRUPT > 29
	static ESP_ISR void isr29(void);
	#endif
#if CORE_NUM_INTERRUPT > 30
	static ESP_ISR void isr30(void);
	#endif
#if CORE_NUM_INTERRUPT > 31
	static ESP_ISR void isr31(void);
	#endif
#if CORE_NUM_INTERRUPT > 32
	static ESP_ISR void isr32(void);
	#endif
#if CORE_NUM_INTERRUPT > 33
	static ESP_ISR void isr33(void);
	#endif
#if CORE_NUM_INTERRUPT > 34
	static ESP_ISR void isr34(void);
	#endif
#if CORE_NUM_INTERRUPT > 35
	static ESP_ISR void isr35(void);
	#endif
#if CORE_NUM_INTERRUPT > 36
	static ESP_ISR void isr36(void);
	#endif
#if CORE_NUM_INTERRUPT > 37
	static ESP_ISR void isr37(void);
	#endif
#if CORE_NUM_INTERRUPT > 38
	static ESP_ISR void isr38(void);
	#endif
#if CORE_NUM_INTERRUPT > 39
	static ESP_ISR void isr39(void);
	#endif
#if CORE_NUM_INTERRUPT > 40
	static ESP_ISR void isr40(void);
#endif
#if CORE_NUM_INTERRUPT > 41
	static ESP_ISR void isr41(void);
#endif
#if CORE_NUM_INTERRUPT > 42
	static ESP_ISR void isr42(void);
#endif
#if CORE_NUM_INTERRUPT > 43
	static ESP_ISR void isr43(void);
#endif
#if CORE_NUM_INTERRUPT > 44
	static ESP_ISR void isr44(void);
#endif
#if CORE_NUM_INTERRUPT > 45
	static ESP_ISR void isr45(void);
#endif
#if CORE_NUM_INTERRUPT > 46
	static ESP_ISR void isr46(void);
#endif
#if CORE_NUM_INTERRUPT > 47
	static ESP_ISR void isr47(void);
#endif
#if CORE_NUM_INTERRUPT > 48
	static ESP_ISR void isr48(void);
#endif
#if CORE_NUM_INTERRUPT > 49
	static ESP_ISR void isr49(void);
#endif
#if CORE_NUM_INTERRUPT > 50
	static ESP_ISR void isr50(void);
#endif
#if CORE_NUM_INTERRUPT > 51
	static ESP_ISR void isr51(void);
#endif
#if CORE_NUM_INTERRUPT > 52
	static ESP_ISR void isr52(void);
#endif
#if CORE_NUM_INTERRUPT > 53
	static ESP_ISR void isr53(void);
#endif
#if CORE_NUM_INTERRUPT > 54
	static ESP_ISR void isr54(void);
#endif
#if CORE_NUM_INTERRUPT > 55
	static ESP_ISR void isr55(void);
#endif
#if CORE_NUM_INTERRUPT > 56
	static ESP_ISR void isr56(void);
#endif
#if CORE_NUM_INTERRUPT > 57
	static ESP_ISR void isr57(void);
#endif
#if CORE_NUM_INTERRUPT > 58
	static ESP_ISR void isr58(void);
#endif
#if CORE_NUM_INTERRUPT > 59
	static ESP_ISR void isr59(void);
#endif
#if CORE_NUM_INTERRUPT > 60
	static ESP_ISR void isr60(void);
#endif
#if CORE_NUM_INTERRUPT > 61
	static ESP_ISR void isr61(void);
#endif
#if CORE_NUM_INTERRUPT > 62
	static ESP_ISR void isr62(void);
#endif
#if CORE_NUM_INTERRUPT > 63
	static ESP_ISR void isr63(void);
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
#endif /* NEWENCODER_H_ */
