#include "Arduino.h"
#include "NewEncoder.h"

// Pins 2 and 3 should work for many processors, including Uno. See README for meaning of constructor arguments.
// However, for polling, we use pins without interrupt support.
// Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
// Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.mouser.com/ProductDetail/alps/ec11e15244g1/?qs=YMSFtX0bdJDiV4LBO61anw==&countrycode=US&currencycode=USD

#define PIN_A 4
#define PIN_B 5

volatile int16_t pollingBuffer = 0xFF;
NewEncoder encoder(PIN_A, PIN_B, -20, 20, 0, FULL_PULSE, (uint8_t *)pollingBuffer);
int16_t prevEncoderValue;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);

  delay(10);

  NewEncoder::EncoderState state;
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

void updatePollingBuffer() {
  // Fill the polling buffer
  // This can be more efficient with a shift register or i/o expander.
  uint16_t valueA = digitalRead(PIN_A);
  uint16_t valueB = digitalRead(PIN_B);

  pollingBuffer = (pollingBuffer & ~(0b1 << PIN_A)) | (valueA << PIN_A);
  pollingBuffer = (pollingBuffer & ~(0b1 << PIN_B)) | (valueB << PIN_B);

  // read data from the input buffer, and update its linked encoder.
  encoder.pollInput();
}

void loop() {
  updatePollingBuffer();

  int16_t currentValue;
  NewEncoder::EncoderState currentEncoderState;

  if (encoder.getState(currentEncoderState)) {
    Serial.print("Encoder: ");
    currentValue = currentEncoderState.currentValue;
    if (currentValue != prevEncoderValue) {
      Serial.println(currentValue);
      prevEncoderValue = currentValue;
    } else
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
