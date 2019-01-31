/*
 * NewEncoder.h
 */
#ifndef NEWENCODER_H_
#define NEWENCODER_H_

#include <Arduino.h>
#include "utility\interrupt_pins.h"
#include "utility\direct_pin_read.h"

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
	NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type =
			FULL_PULSE);
	NewEncoder();
	~NewEncoder();
	bool begin();
	void configure(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type =
			FULL_PULSE);
	void end();
	bool enabled();
	int16_t setValue(int16_t);
	int16_t getValue();
	int16_t operator=(int16_t val);
	int16_t getAndSet(int16_t val = 0);
	operator int16_t() const;
	bool upClick();
	bool downClick();

private:
	void pinChangeHandler(uint8_t index);
	void aPinChange();
	void bPinChange();
	bool active = false;
	bool configured = false;
	uint8_t _aPin = 0, _bPin = 0;
	volatile int16_t _minValue = 0, _maxValue = 0;
	//int16_t _interruptA = -1, _interruptB = -1;
	const encoderStateTransition *tablePtr = nullptr;
	volatile uint8_t _aPinValue, _bPinValue;
	volatile uint8_t _currentState;
	volatile int16_t _currentValue;
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

#if CORE_NUM_INTERRUPT > 0
	static void isr00(void) {
		CALL_MEMBER_FN(_isrTable[0].objectPtr, _isrTable[0].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 1
	static void isr01(void) {
		CALL_MEMBER_FN(_isrTable[1].objectPtr, _isrTable[1].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 2
	static void isr02(void) {
		CALL_MEMBER_FN(_isrTable[2].objectPtr, _isrTable[2].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 3
	static void isr03(void) {
		CALL_MEMBER_FN(_isrTable[3].objectPtr, _isrTable[3].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 4
	static void isr04(void) {
		CALL_MEMBER_FN(_isrTable[4].objectPtr, _isrTable[4].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 5
	static void isr05(void) {
		CALL_MEMBER_FN(_isrTable[5].objectPtr, _isrTable[5].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 6
	static void isr06(void) {
		CALL_MEMBER_FN(_isrTable[6].objectPtr, _isrTable[6].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 7
	static void isr07(void) {
		CALL_MEMBER_FN(_isrTable[7].objectPtr, _isrTable[7].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 8
	static void isr08(void) {
		CALL_MEMBER_FN(_isrTable[8].objectPtr, _isrTable[8].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 9
	static void isr09(void) {
		CALL_MEMBER_FN(_isrTable[9].objectPtr, _isrTable[9].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 10
	static void isr10(void) {
		CALL_MEMBER_FN(_isrTable[10].objectPtr, _isrTable[10].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 11
	static void isr11(void) {
		CALL_MEMBER_FN(_isrTable[11].objectPtr, _isrTable[11].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 12
	static void isr12(void) {
		CALL_MEMBER_FN(_isrTable[12].objectPtr, _isrTable[12].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 13
	static void isr13(void) {
		CALL_MEMBER_FN(_isrTable[13].objectPtr, _isrTable[13].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 14
	static void isr14(void) {
		CALL_MEMBER_FN(_isrTable[14].objectPtr, _isrTable[14].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 15
	static void isr15(void) {
		CALL_MEMBER_FN(_isrTable[15].objectPtr, _isrTable[15].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 16
	static void isr16(void) {
		CALL_MEMBER_FN(_isrTable[16].objectPtr, _isrTable[16].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 17
	static void isr17(void) {
		CALL_MEMBER_FN(_isrTable[17].objectPtr, _isrTable[17].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 18
	static void isr18(void) {
		CALL_MEMBER_FN(_isrTable[18].objectPtr, _isrTable[18].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 19
	static void isr19(void) {
		CALL_MEMBER_FN(_isrTable[19].objectPtr, _isrTable[19].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 20
	static void isr20(void) {
		CALL_MEMBER_FN(_isrTable[20].objectPtr, _isrTable[20].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 21
	static void isr21(void) {
		CALL_MEMBER_FN(_isrTable[21].objectPtr, _isrTable[21].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 22
	static void isr22(void) {
		CALL_MEMBER_FN(_isrTable[22].objectPtr, _isrTable[22].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 23
	static void isr23(void) {
		CALL_MEMBER_FN(_isrTable[23].objectPtr, _isrTable[23].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 24
	static void isr24(void) {
		CALL_MEMBER_FN(_isrTable[24].objectPtr, _isrTable[24].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 25
	static void isr25(void) {
		CALL_MEMBER_FN(_isrTable[25].objectPtr, _isrTable[25].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 26
	static void isr26(void) {
		CALL_MEMBER_FN(_isrTable[26].objectPtr, _isrTable[26].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 27
	static void isr27(void) {
		CALL_MEMBER_FN(_isrTable[27].objectPtr, _isrTable[27].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 28
	static void isr28(void) {
		CALL_MEMBER_FN(_isrTable[28].objectPtr, _isrTable[28].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 29
	static void isr29(void) {
		CALL_MEMBER_FN(_isrTable[29].objectPtr, _isrTable[29].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 30
	static void isr30(void) {
		CALL_MEMBER_FN(_isrTable[30].objectPtr, _isrTable[30].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 31
	static void isr31(void) {
		CALL_MEMBER_FN(_isrTable[31].objectPtr, _isrTable[31].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 32
	static void isr32(void) {
		CALL_MEMBER_FN(_isrTable[32].objectPtr, _isrTable[32].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 33
	static void isr33(void) {
		CALL_MEMBER_FN(_isrTable[33].objectPtr, _isrTable[33].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 34
	static void isr34(void) {
		CALL_MEMBER_FN(_isrTable[34].objectPtr, _isrTable[34].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 35
	static void isr35(void) {
		CALL_MEMBER_FN(_isrTable[35].objectPtr, _isrTable[35].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 36
	static void isr36(void) {
		CALL_MEMBER_FN(_isrTable[36].objectPtr, _isrTable[36].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 37
	static void isr37(void) {
		CALL_MEMBER_FN(_isrTable[37].objectPtr, _isrTable[37].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 38
	static void isr38(void) {
		CALL_MEMBER_FN(_isrTable[38].objectPtr, _isrTable[38].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 39
	static void isr39(void) {
		CALL_MEMBER_FN(_isrTable[39].objectPtr, _isrTable[39].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 40
	static void isr40(void) {
		CALL_MEMBER_FN(_isrTable[40].objectPtr, _isrTable[40].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 41
	static void isr41(void) {
		CALL_MEMBER_FN(_isrTable[41].objectPtr, _isrTable[41].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 42
	static void isr42(void) {
		CALL_MEMBER_FN(_isrTable[42].objectPtr, _isrTable[42].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 43
	static void isr43(void) {
		CALL_MEMBER_FN(_isrTable[43].objectPtr, _isrTable[43].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 44
	static void isr44(void) {
		CALL_MEMBER_FN(_isrTable[44].objectPtr, _isrTable[44].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 45
	static void isr45(void) {
		CALL_MEMBER_FN(_isrTable[45].objectPtr, _isrTable[45].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 46
	static void isr46(void) {
		CALL_MEMBER_FN(_isrTable[46].objectPtr, _isrTable[46].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 47
	static void isr47(void) {
		CALL_MEMBER_FN(_isrTable[47].objectPtr, _isrTable[47].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 48
	static void isr48(void) {
		CALL_MEMBER_FN(_isrTable[48].objectPtr, _isrTable[48].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 49
	static void isr49(void) {
		CALL_MEMBER_FN(_isrTable[49].objectPtr, _isrTable[49].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 50
	static void isr50(void) {
		CALL_MEMBER_FN(_isrTable[50].objectPtr, _isrTable[50].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 51
	static void isr51(void) {
		CALL_MEMBER_FN(_isrTable[51].objectPtr, _isrTable[51].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 52
	static void isr52(void) {
		CALL_MEMBER_FN(_isrTable[52].objectPtr, _isrTable[52].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 53
	static void isr53(void) {
		CALL_MEMBER_FN(_isrTable[53].objectPtr, _isrTable[53].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 54
	static void isr54(void) {
		CALL_MEMBER_FN(_isrTable[54].objectPtr, _isrTable[54].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 55
	static void isr55(void) {
		CALL_MEMBER_FN(_isrTable[55].objectPtr, _isrTable[55].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 56
	static void isr56(void) {
		CALL_MEMBER_FN(_isrTable[56].objectPtr, _isrTable[56].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 57
	static void isr57(void) {
		CALL_MEMBER_FN(_isrTable[57].objectPtr, _isrTable[57].functPtr);
	}
#endif
#if CORE_NUM_INTERRUPT > 58
	static void isr58(void) {
		CALL_MEMBER_FN(_isrTable[58].objectPtr, _isrTable[58].functPtr);
	}
#endif
};

#endif /* NEWENCODER_H_ */
