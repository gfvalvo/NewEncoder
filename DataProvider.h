/*
 * DataProvider.h
 */
#ifndef DATAPROVIDER_H_
#define DATAPROVIDER_H_

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

#include "utility/interrupt_pins.h"
#include "utility/direct_pin_read.h"
// #define IO_REG_TYPE			uint8_t
// #define PIN_TO_BASEREG(pin)             (portInputRegister(digitalPinToPort(pin)))
// #define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
// #define DIRECT_PIN_READ(base, mask)     (((*(base)) & (mask)) ? 1 : 0)

class DataConsumer {
public:
	virtual void ESP_ISR checkPinChange(uint8_t index) = 0;
};

class DataProvider {

public:
	DataProvider(uint8_t aPin, uint8_t bPin, DataConsumer *target);
	DataProvider();
	virtual ~DataProvider();
	virtual bool begin();
	virtual void configure(uint8_t aPin, uint8_t bPin, DataConsumer *target);
	virtual void end();

	uint8_t aPinValue() const { return _aPinValue; };
	uint8_t bPinValue() const { return _bPinValue; };

private:
	void aPinChange();
	void bPinChange();
	DataConsumer *_target;
	
	uint8_t _aPin = 0, _bPin = 0;
	volatile uint8_t _aPinValue, _bPinValue;
	volatile IO_REG_TYPE *_aPin_register;
	volatile IO_REG_TYPE *_bPin_register;
	volatile IO_REG_TYPE _aPin_bitmask;
	volatile IO_REG_TYPE _bPin_bitmask;

#ifndef USE_FUNCTIONAL_ISR
	using PinChangeFunction = void (DataProvider::*)();
	using isrFunct = void (*)();

	struct isrInfo {
		DataProvider *objectPtr;
		PinChangeFunction functPtr;
	};
	static isrInfo _isrTable[CORE_NUM_INTERRUPT];

	template<uint8_t NUM_INTERRUPTS = CORE_NUM_INTERRUPT>
	static isrFunct getIsr(uint8_t intNumber);
#endif
};

#ifndef USE_FUNCTIONAL_ISR
template<uint8_t NUM_INTERRUPTS>
DataProvider::isrFunct DataProvider::getIsr(uint8_t intNumber) {
	if (intNumber == (NUM_INTERRUPTS - 1)) {
		return [] {
			((_isrTable[NUM_INTERRUPTS - 1].objectPtr)->*(_isrTable[NUM_INTERRUPTS - 1].functPtr))();
		};
	}
	return getIsr<NUM_INTERRUPTS - 1>(intNumber);
}

template<>
inline DataProvider::isrFunct DataProvider::getIsr<0>(uint8_t intNum) {
	(void) intNum;
	return nullptr;
}
#endif

#endif /* DATAPROVIDER_H_ */