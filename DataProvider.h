/*
 * InterruptDataProvider.h
 */
#ifndef DATAPROVIDER_H_
#define DATAPROVIDER_H_

#include <Arduino.h>

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
	friend class InterruptDataProvider;
protected:
	virtual void ESP_ISR checkPinChange(uint8_t index) = 0;
};

class DataProvider {
public:
	static DataProvider* createDefault() { return DataProvider::createInterruptDataProvider(); };
	static DataProvider* createInterruptDataProvider();
	
public:
	DataProvider(uint8_t aPin, uint8_t bPin, DataConsumer *target) {};
	DataProvider() {};
	virtual ~DataProvider() {};
	virtual bool begin() = 0;
	virtual void configure(uint8_t aPin, uint8_t bPin, DataConsumer *target) = 0;
	virtual void end() = 0;

	virtual void interruptOn() const = 0;
	virtual void interruptOff() const = 0;

	uint8_t aPinValue() const { return _aPinValue; };
	uint8_t bPinValue() const { return _bPinValue; };

	DataProvider(const DataProvider&) = delete; // delete copy constructor. no copying allowed
	virtual DataProvider& operator=(const DataProvider&) = delete; // delete operator=(). no assignment allowed

protected:
	DataConsumer *_target;
	
	uint8_t _aPin = 0, _bPin = 0;
	volatile uint8_t _aPinValue, _bPinValue;
	volatile IO_REG_TYPE *_aPin_register;
	volatile IO_REG_TYPE *_bPin_register;
	volatile IO_REG_TYPE _aPin_bitmask;
	volatile IO_REG_TYPE _bPin_bitmask;
};

#endif /* DATAPROVIDER_H_ */