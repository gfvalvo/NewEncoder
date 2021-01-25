#include "Arduino.h"
#include "NewEncoder.h"

// Demonstrate creation of custom Encoder using inheritance.
// This encoder "wraps around" when it hits the min or max limit

class CustomEncoder: public NewEncoder {
  public:
    CustomEncoder() :
      NewEncoder() {
    }
    CustomEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE) :
      NewEncoder(aPin, bPin, minValue, maxValue, initalValue, type) {
    }
    virtual ~CustomEncoder() {
    }

  protected:
    virtual void updateValue(uint8_t updatedState);
};

void ESP_ISR CustomEncoder::updateValue(uint8_t updatedState) {
  if ((updatedState & DELTA_MASK) == INCREMENT_DELTA) {
    liveState.currentClick = UpClick;
    liveState.currentValue++;
    if (liveState.currentValue > _maxValue) {
      liveState.currentValue = _minValue;
    }
  } else if ((updatedState & DELTA_MASK) == DECREMENT_DELTA) {
    liveState.currentClick = DownClick;
    liveState.currentValue--;
    if (liveState.currentValue < _minValue) {
      liveState.currentValue = _maxValue;
    }
  }
  stateChanged = true;
}

// Pins 2 and 3 should work for many processors, including Uno. See README for meaning of constructor arguments.
// Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
// Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.mouser.com/ProductDetail/alps/ec11e15244g1/?qs=YMSFtX0bdJDiV4LBO61anw==&countrycode=US&currencycode=USD
CustomEncoder encoder(2, 3, 0, 20, 10, FULL_PULSE);
int16_t prevEncoderValue;

void setup() {
  CustomEncoder::EncoderState state;

  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");
  if (!encoder.begin()) {
    Serial.println("Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.");
    while (1) {
      yield();
    }
  } else {
    encoder.getState(state);
    Serial.print("Encoder Successfully Started at value = ");
    prevEncoderValue = state.currentValue;
    Serial.println(prevEncoderValue);
  }
}

void loop() {
  int16_t currentValue;
  CustomEncoder::EncoderState currentEncoderState;

  if (encoder.getState(currentEncoderState)) {
    currentValue = currentEncoderState.currentValue;
    if (currentValue != prevEncoderValue) {
      Serial.print("Encoder: ");
      Serial.println(currentValue);
      prevEncoderValue = currentValue;
    }
  }
}
