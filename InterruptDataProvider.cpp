/*
 * InterruptDataProvider.cpp
 */

#include "InterruptDataProvider.h"

#ifndef USE_FUNCTIONAL_ISR
InterruptDataProvider::isrInfo InterruptDataProvider::_isrTable[CORE_NUM_INTERRUPT];
#endif

InterruptDataProvider::InterruptDataProvider(uint8_t aPin, uint8_t bPin, DataConsumer *target) {
	configure(aPin, bPin, target);
}

InterruptDataProvider::InterruptDataProvider() {
	_aPin_register = nullptr;
	_bPin_register = nullptr;
	_target = nullptr;
}

InterruptDataProvider::~InterruptDataProvider() {
	end();
}

void InterruptDataProvider::end() {
	int16_t _interruptA = digitalPinToInterrupt(_aPin);
	int16_t _interruptB = digitalPinToInterrupt(_bPin);
	detachInterrupt(_interruptA);
	detachInterrupt(_interruptB);
}

void InterruptDataProvider::configure(uint8_t aPin, uint8_t bPin, DataConsumer *target) {
	_aPin = aPin;
	_bPin = bPin;
	_aPin_register = PIN_TO_BASEREG(aPin);
	_bPin_register = PIN_TO_BASEREG(bPin);
	_aPin_bitmask = PIN_TO_BITMASK(aPin);
	_bPin_bitmask = PIN_TO_BITMASK(bPin);
	_target = target;
}

bool InterruptDataProvider::begin() {
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

	pinMode(_aPin, INPUT_PULLUP);
	pinMode(_bPin, INPUT_PULLUP);
	delay(2);  // Seems to help ensure first reading after pinMode is correct
	_aPinValue = DIRECT_PIN_READ(_aPin_register, _aPin_bitmask);
	_bPinValue = DIRECT_PIN_READ(_bPin_register, _bPin_bitmask);

#ifndef USE_FUNCTIONAL_ISR
	_isrTable[_interruptA].objectPtr = this;
	_isrTable[_interruptA].functPtr = &InterruptDataProvider::onAPinChange;
	auto isrA = getIsr(_interruptA);
	if (isrA == nullptr) {
		return false;
	}

	_isrTable[_interruptB].objectPtr = this;
	_isrTable[_interruptB].functPtr = &InterruptDataProvider::onBPinChange;
	auto isrB = getIsr(_interruptB);
	if (isrB == nullptr) {
		return false;
	}

	attachInterrupt(_interruptA, isrA, CHANGE);
	attachInterrupt(_interruptB, isrB, CHANGE);

#else
	auto aPinIsr = [this] {
		this->onAPinChange();
	};
	attachInterrupt(_interruptA, aPinIsr, CHANGE);

	auto bPinIsr = [this] {
		this->onBPinChange();
	};
	attachInterrupt(_interruptB, bPinIsr, CHANGE);

#endif
	return true;
}

void ESP_ISR InterruptDataProvider::onAPinChange() {
	this->aPinChange(DIRECT_PIN_READ(_aPin_register, _aPin_bitmask));
}

void ESP_ISR InterruptDataProvider::onBPinChange() {
	this->bPinChange(DIRECT_PIN_READ(_bPin_register, _bPin_bitmask));
}

void InterruptDataProvider::interruptOn() const { 
	interrupts(); 
}

void InterruptDataProvider::interruptOff() const {
	noInterrupts();
}

DataProvider* DataProvider::createInterruptDataProvider() {
	return new InterruptDataProvider();
}
