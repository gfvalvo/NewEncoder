/*
 * PollingDataProvider.cpp
 */

#include "PollingDataProvider.h"

#define POLLING_PIN_TO_BASEREG(pin)             ((this->_input_buffer) + ((pin) / 8))
#define POLLING_PIN_TO_BITMASK(pin)             (1 << ((pin) % 8))
#define POLLING_DIRECT_PIN_READ(base, mask)     (((*(base)) & (mask)) ? 1 : 0)

PollingDataProvider::PollingDataProvider(uint8_t aPin, uint8_t bPin, DataConsumer *target, uint8_t *input_buffer)
    : PollingDataProvider(input_buffer) {
	configure(aPin, bPin, target);
}

PollingDataProvider::PollingDataProvider(uint8_t *input_buffer): _input_buffer(input_buffer) {
	_aPin_register = nullptr;
	_bPin_register = nullptr;
	_target = nullptr;
}

PollingDataProvider::~PollingDataProvider() {
	end();
}

void PollingDataProvider::end() {
}

void PollingDataProvider::configure(uint8_t aPin, uint8_t bPin, DataConsumer *target) {
	_aPin = aPin;
	_bPin = bPin;
	_aPin_register = POLLING_PIN_TO_BASEREG(aPin);
	_bPin_register = POLLING_PIN_TO_BASEREG(bPin);
	_aPin_bitmask = POLLING_PIN_TO_BITMASK(aPin);
	_bPin_bitmask = POLLING_PIN_TO_BITMASK(bPin);
	_target = target;
}

bool PollingDataProvider::begin() {
	if (_aPin == _bPin) {
		return false;
	}

	delay(2);  // Seems to help ensure first reading after pinMode is correct
	_aPinValue = POLLING_DIRECT_PIN_READ(_aPin_register, _aPin_bitmask);
	_bPinValue = POLLING_DIRECT_PIN_READ(_bPin_register, _bPin_bitmask);

	return true;
}

void PollingDataProvider::aPinChange() {
	uint8_t newPinValue = POLLING_DIRECT_PIN_READ(_aPin_register, _aPin_bitmask);
	if (newPinValue == _aPinValue) {
		return;
	}
	_aPinValue = newPinValue;
	if (_target == nullptr) return;
	_target->checkPinChange(0b00 | _aPinValue);  // Falling aPin == 0b00, Rising aPin = 0b01;
}

void PollingDataProvider::bPinChange() {
	uint8_t newPinValue = POLLING_DIRECT_PIN_READ(_bPin_register, _bPin_bitmask);
	if (newPinValue == _bPinValue) {
		return;
	}
	_bPinValue = newPinValue;
	if (_target == nullptr) return;
	_target->checkPinChange(0b10 | _bPinValue);  // Falling bPin == 0b10, Rising bPin = 0b11;
}

DataProvider* DataProvider::createPollingDataProvider(uint8_t *input_buffer) {
	return new PollingDataProvider(input_buffer);
}
