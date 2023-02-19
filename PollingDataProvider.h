#ifndef POLLING_DATA_PROVIDER_H_
#define POLLING_DATA_PROVIDER_H_

#include "DataProvider.h"

class PollingDataProvider: public DataProvider {
public:
	PollingDataProvider(uint8_t aPin, uint8_t bPin, DataConsumer *target, uint8_t *input_buffer);
	PollingDataProvider(uint8_t *input_buffer);
	virtual ~PollingDataProvider() override;
	virtual bool begin() override;
	virtual void configure(uint8_t aPin, uint8_t bPin, DataConsumer *target) override;
	virtual void end() override;

	virtual void interruptOn() const override {};
	virtual void interruptOff() const override {};

	uint8_t aPinValue() const { return _aPinValue; };
	uint8_t bPinValue() const { return _bPinValue; };

    virtual void inputUpdate() override { aPinChange(); bPinChange(); };

	PollingDataProvider(const DataProvider&) = delete; // delete copy constructor. no copying allowed
	virtual PollingDataProvider& operator=(const DataProvider&) override = delete; // delete operator=(). no assignment allowed

private:
	void aPinChange();
	void bPinChange();
    uint8_t *_input_buffer;
};

#endif /* POLLING_DATA_PROVIDER_H_ */