#include "Arduino.h"
#include "NewEncoder.h"

// Adjust number of encoders and pin assignments for particular processor. These work for Teensy 3.2. See README for meaning of constructor arguments.
// Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
// Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.mouser.com/ProductDetail/alps/ec11e15244g1/?qs=YMSFtX0bdJDiV4LBO61anw==&countrycode=US&currencycode=USD
NewEncoder encoders[] = {
  { 0, 1, -20, 20, 0, HALF_PULSE },
  { 20, 21, 0, 50, 25, FULL_PULSE },
  { 5, 6, -25, 0, -13, HALF_PULSE },
  { 11, 12, -10, 25, 8, FULL_PULSE }
};

const uint8_t numEncoders = sizeof(encoders) / sizeof(encoders[0]);
int16_t prevEncoderValue[numEncoders];

void setup() {
  NewEncoder::EncoderState state;

  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");

  for (uint8_t index = 0; index < numEncoders; index++) {
    if (!encoders[index].begin()) {
      Serial.print("Encoder: ");
      Serial.print(index);
      Serial.println(" Failed to Start. Check pin assignments and available interrupts. Aborting.");
      while (1) {
        yield();
      }
    } else {
      encoders[index].getState(state);
      prevEncoderValue[index] = state.currentValue;
      Serial.print("Encoder: ");
      Serial.print(index);
      Serial.print(" Successfully Started at value = ");
      Serial.println(state.currentValue);
    }
  }
}

void loop() {
  int16_t currentValue;
  NewEncoder::EncoderState currentEncoderState;
  for (uint8_t index = 0; index < numEncoders; index++) {
    if (encoders[index].getState(currentEncoderState)) {
      Serial.print("Encoder ");
      Serial.print(index);
      Serial.print(": ");
      currentValue = currentEncoderState.currentValue;
      if (currentValue != prevEncoderValue[index]) {
        Serial.println(currentValue);
        prevEncoderValue[index] = currentValue;
      } else {
        switch (currentEncoderState.currentClick) {
          case NewEncoder::UpClick:
            Serial.println("at upper limit.");
            break;

          case NewEncoder::DownClick:
            Serial.println("at lower limit.");
            break;

          default:
            break;
        }
      }
    }
  }
}
