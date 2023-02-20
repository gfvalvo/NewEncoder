
# About polling mode
Initially, the NewEncoder instance uses only interrupts to handle the data input from encoder pins.
However, on some board, only a few pins support interrupt.
The polling mode allows the encoder to work with a input buffer, therefore we can hook more encoders.

## The data providers
Here we have two kinds of data provider for woking in interrupt mode and polling mode.
-  `InterruptDataProvider`
-  `PollingDataProvider`

The constructor of `NewEncoder` accepts a data provider instance as its last parameter.  If it is `nullptr`, the `InterruptDataProvider` is used by default.
```
NewEncoder(uint8_t  aPin, uint8_t  bPin, int16_t  minValue, int16_t maxValue, int16_t initalValue, uint8_t  type = FULL_PULSE, DataProvider *provider = nullptr);
```
To initialize the encoder instance with data provider explicitly specified, here's how
```
// To create an encoder with interrupt data provider
DataProvider *provider = DataProvider::createInterruptDataProvider();
NewEncoder encoder1(2, 3, -20, 20, 0, FULL_PULSE, provider);

// To create an encoder with interrupt data provider
volatile uint32_t buffer = 0xFF; // the buffer can be any size, and usually the initial value for all bits should be "pulled up" to 1.
DataProvider *pollingProvider = DataProvider::createPollingDataProvider((uint8_t *)buffer);
NewEncoder encoder2(2, 3, -20, 20, 0, FULL_PULSE, pollingProvider);
```
**NOTES:**
The ownership of data provider is not transfered to the encoder instance, if needed, you have to `delete` them manually.

## Update encoder in polling mode
With pulling mode, we need to call `DataProvider::inputUpdate()` to notify its encoder to check the latest state.
```
buffer = getLatestValue();
pollingProvider->inputUpdate();
```

For details, please check the sample sketch located at `examples/Polling/PollingSingleEncoder.ino`.
