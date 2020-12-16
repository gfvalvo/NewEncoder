#include "Arduino.h"
#include "NewEncoder.h"

void ESP_ISR callBack(NewEncoder &enc);

// Pins 2 and 3 should work for many processors, including Uno. See README for meaning of constructor arguments.
// Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
// Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.adafruit.com/product/377
NewEncoder encoder(2, 3, -20, 20, 0, FULL_PULSE);

volatile int16_t encoderValue;
volatile bool newValue;

void setup() {
  int16_t value;

  Serial.begin(115200);
  delay(2000);
  Serial.println(F("Starting"));
  if (!encoder.begin()) {

    Serial.println(F("Encoder Failed to Start. Check pin assignments and available interrupts. Aborting."));
    while (1) {
    }
  } else {
#if defined(__AVR__)
    noInterrupts();  // 16-bit access not atomic on 8-bit processor
#endif
    encoderValue = encoder;
    value = encoderValue;
#if defined(__AVR__)
    interrupts();
#endif
    Serial.print(F("Encoder Successfully Started at value = "));
    Serial.println(value);
  }
  encoder.attachCallback(callBack);
}

void loop() {
  int16_t currentEncoderValue;

  if (newValue) {
    noInterrupts();
    newValue = false;
    currentEncoderValue = encoderValue;
    interrupts();
    Serial.print("Encoder: ");
    Serial.println(currentEncoderValue);
  }
}

void ESP_ISR callBack(NewEncoder &enc) {
  int16_t e = enc;
  if (e != encoderValue) {
    encoderValue = e;
    newValue = true;
  }
}
