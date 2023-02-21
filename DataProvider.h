/*
 * DataProvider.h
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

class DataConsumer {
	friend class DataProvider;
protected:
	DataProvider *dataProvider = nullptr;
	virtual void checkPinChange(uint8_t index) = 0;
};

class DataProvider {
public:
	static DataProvider* createInterruptDataProvider();
	static DataProvider* createPollingDataProvider(uint8_t* input_buffer);
	
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

	virtual void inputUpdate() {}; // force to check the pin change, only used in polling mode

protected:
	void aPinChange(uint8_t newPinValue) {
		if (newPinValue == _aPinValue) {
			return;
		}
		_aPinValue = newPinValue;
		if (_target == nullptr) return;
		_target->checkPinChange(0b00 | _aPinValue);  // Falling aPin == 0b00, Rising aPin = 0b01;
	};

	void bPinChange(uint8_t newPinValue) {
		if (newPinValue == _bPinValue) {
			return;
		}
		_bPinValue = newPinValue;
		if (_target == nullptr) return;
		_target->checkPinChange(0b10 | _bPinValue);  // Falling bPin == 0b10, Rising bPin = 0b11;
	}

protected:
	DataConsumer *_target = nullptr;
	
	uint8_t _aPin = 0, _bPin = 0;
	volatile uint8_t _aPinValue, _bPinValue;
	volatile IO_REG_TYPE *_aPin_register;
	volatile IO_REG_TYPE *_bPin_register;
	volatile IO_REG_TYPE _aPin_bitmask;
	volatile IO_REG_TYPE _bPin_bitmask;
};

#endif /* DATAPROVIDER_H_ */
