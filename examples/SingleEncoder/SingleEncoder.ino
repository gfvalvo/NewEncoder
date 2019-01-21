/*
   SingleEncoder.cpp

    Created on: Jul 25, 2018
        Author: GFV
*/
#include "Arduino.h"
#include "NewEncoder.h"

// Pins 2 and 3 should work for many processors, including Uno
NewEncoder encoder(2, 3, -20, 20, 0, FULL_PULSE);
int16_t prevEncoderValue;

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
    value = encoder;
    Serial.print(F("Encoder Successfully Started at value = "));
    Serial.println(value);
  }
}

void loop() {
  int16_t currentValue;
  bool up, down;

  up = encoder.upClick();
  down = encoder.downClick();
  if (up || down) {
    currentValue = encoder;
    if (currentValue != prevEncoderValue) {
      Serial.print(F("Encoder: "));
      Serial.println(currentValue);
      prevEncoderValue = currentValue;
    } else if (up) {
      Serial.println(F("Encoder at upper limit."));
    } else {
      Serial.println(F("Encoder at lower limit."));
    }
  }
}
