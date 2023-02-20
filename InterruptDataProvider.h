#ifndef INTERRUPT_DATA_PROVIDER_H_
#define INTERRUPT_DATA_PROVIDER_H_

#include "DataProvider.h"

class InterruptDataProvider: public DataProvider {
public:
	InterruptDataProvider(uint8_t aPin, uint8_t bPin, DataConsumer *target);
	InterruptDataProvider();
	virtual ~InterruptDataProvider() override;
	virtual bool begin() override;
	virtual void configure(uint8_t aPin, uint8_t bPin, DataConsumer *target) override;
	virtual void end() override;

	virtual void interruptOn() const override;
	virtual void interruptOff() const override;

	InterruptDataProvider(const DataProvider&) = delete; // delete copy constructor. no copying allowed
	virtual DataProvider& operator=(const DataProvider&) override = delete; // delete operator=(). no assignment allowed

private:
	void onAPinChange();
	void onBPinChange();

#ifndef USE_FUNCTIONAL_ISR
	using PinChangeFunction = void (InterruptDataProvider::*)();
	using isrFunct = void (*)();

	struct isrInfo {
		InterruptDataProvider *objectPtr;
		PinChangeFunction functPtr;
	};
	static isrInfo _isrTable[CORE_NUM_INTERRUPT];

	template<uint8_t NUM_INTERRUPTS = CORE_NUM_INTERRUPT>
	static isrFunct getIsr(uint8_t intNumber);
#endif
};

#ifndef USE_FUNCTIONAL_ISR
template<uint8_t NUM_INTERRUPTS>
InterruptDataProvider::isrFunct InterruptDataProvider::getIsr(uint8_t intNumber) {
	if (intNumber == (NUM_INTERRUPTS - 1)) {
		return [] {
			((_isrTable[NUM_INTERRUPTS - 1].objectPtr)->*(_isrTable[NUM_INTERRUPTS - 1].functPtr))();
		};
	}
	return getIsr<NUM_INTERRUPTS - 1>(intNumber);
}

template<>
inline InterruptDataProvider::isrFunct InterruptDataProvider::getIsr<0>(uint8_t intNum) {
	(void) intNum;
	return nullptr;
}
#endif

#endif /* INTERRUPT_DATA_PROVIDER_H_ */