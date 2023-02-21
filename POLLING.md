
# About polling mode
Initially, the NewEncoder library uses only interrupts to handle the data input from encoder pins.
However, on some board, only a few pins support interrupt.
The polling mode allows the encoder to work with a input buffer, therefore we can hook more encoders.

## The data providers
Here we have two kinds of data provider for woking in interrupt mode and polling mode.
-  `InterruptDataProvider`
-  `PollingDataProvider`

The constructor of `NewEncoder` accepts the pointer to a input buffer as its last parameter.  If it is `nullptr`, the `InterruptDataProvider` is used by default. Otherwise, the `PollingDataProvider` is used internally.
```
NewEncoder(uint8_t  aPin, uint8_t  bPin, int16_t  minValue, int16_t maxValue, int16_t initalValue, uint8_t  type = FULL_PULSE, uint8_t *inputBuffer = nullptr);
```
here's the examples.
```
// To create an encoder with interrupt data provider, the following 4 lines work identically.
NewEncoder encoder(2, 3, -20, 20, 0);
NewEncoder encoder(2, 3, -20, 20, 0, FULL_PULSE);
NewEncoder encoder(2, 3, -20, 20, 0, FULL_PULSE, NULL);
NewEncoder encoder(2, 3, -20, 20, 0, FULL_PULSE, nullptr);

// To create an encoder with interrupt data provider
volatile uint32_t buffer = 0xFF; // the buffer can be any size, and usually the initial value for all bits should be "pulled up" to 1.
NewEncoder encoder2(2, 3, -20, 20, 0, FULL_PULSE, (uint8_t *)buffer);
```

## Update encoder in polling mode
With pulling mode, we need to call `DataProvider::pollInput()` to notify its encoder to check the latest state.
```
buffer = getLatestValue();
pollingProvider->pollInput();
```

For details, please check the sample sketch located at `examples/Polling/PollingSingleEncoder.ino`.
