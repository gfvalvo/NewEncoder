#include "Arduino.h"
#include "NewEncoder.h"

#ifndef ESP32
#error ESP32 Only
#endif

void handleEncoder(void *pvParameters);
void ESP_ISR callBack(NewEncoder &enc);
QueueHandle_t encoderQueue;

void ESP_ISR callBack(NewEncoder &enc);

void setup() {
  Serial.begin(115200);
  delay(1000);

  encoderQueue = xQueueCreate(1, sizeof(int16_t));
  xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 800, NULL, 2, NULL, 1);
}

void loop() {
}

void handleEncoder(void *pvParameters) {
  int16_t encoderReading;

  // This example uses Pins 25 & 26 for Encoder. Specify correct pins for your ESP32 / Encoder setup. See README for meaning of constructor arguments.
  // Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
  // Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.mouser.com/ProductDetail/alps/ec11e15244g1/?qs=YMSFtX0bdJDiV4LBO61anw==&countrycode=US&currencycode=USD
  NewEncoder *encoder1 = new NewEncoder(25, 26, -20, 20, 0, FULL_PULSE);
  if (encoder1 == nullptr) {
    Serial.println("Failed to allocate NewEncoder object. Aborting.");
    vTaskDelete(nullptr);
  }

  if (!encoder1->begin()) {
    Serial.println("Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.");
    delete encoder1;
    vTaskDelete(nullptr);
  }
  Serial.print(F("Encoder Successfully Started at value = "));
  Serial.println((int16_t) *encoder1);
  encoder1->attachCallback(callBack);

  for (;;) {
    xQueueReceive(encoderQueue, &encoderReading, portMAX_DELAY);
    Serial.print("Encoder: ");
    Serial.println(encoderReading);
  }
  vTaskDelete(nullptr);
}

void ESP_ISR callBack(NewEncoder &enc) {
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  static int16_t lastReading = 0;

  int16_t currentReading = enc;
  if (currentReading != lastReading) {
    lastReading = currentReading;
    xQueueSendToBackFromISR(encoderQueue, &currentReading, &pxHigherPriorityTaskWoken);
    if (pxHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }
}
