#include "Arduino.h"
#include "NewEncoder.h"

#ifndef ESP32
#error ESP32 Only
#endif

void handleEncoder(void *pvParameters);
void ESP_ISR callBack(NewEncoder *encPtr, const volatile NewEncoder::EncoderState *state, void *uPtr);
QueueHandle_t encoderQueue;
volatile int16_t prevEncoderValue;

void setup() {
  Serial.begin(115200);
  delay(1000);

  BaseType_t success = xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 1900, NULL, 2, NULL, 1);
  if (!success) {
    printf("Failed to create handleEncoder task. Aborting.\n");
    while (1) {
      yield();
    }
  }
}

void loop() {
}

void handleEncoder(void *pvParameters) {
  NewEncoder::EncoderState currentEncoderstate;
  int16_t currentValue;

  encoderQueue = xQueueCreate(1, sizeof(NewEncoder::EncoderState));
  if (encoderQueue == nullptr) {
    printf("Failed to create encoderQueue. Aborting\n");
    vTaskDelete(nullptr);
  }

  // This example uses Pins 25 & 26 for Encoder. Specify correct pins for your ESP32 / Encoder setup. See README for meaning of constructor arguments.
  // Use FULL_PULSE for encoders that produce one complete quadrature pulse per detnet, such as: https://www.adafruit.com/product/377
  // Use HALF_PULSE for endoders that produce one complete quadrature pulse for every two detents, such as: https://www.mouser.com/ProductDetail/alps/ec11e15244g1/?qs=YMSFtX0bdJDiV4LBO61anw==&countrycode=US&currencycode=USD
  NewEncoder *encoder1 = new NewEncoder(25, 26, -20, 20, 0, HALF_PULSE);
  if (encoder1 == nullptr) {
    printf("Failed to allocate NewEncoder object. Aborting.\n");
    vTaskDelete(nullptr);
  }

  if (!encoder1->begin()) {
    printf("Encoder Failed to Start. Check pin assignments and available interrupts. Aborting.\n");
    delete encoder1;
    vTaskDelete(nullptr);
  }

  encoder1->getState(currentEncoderstate);
  prevEncoderValue = currentEncoderstate.currentValue;
  printf("Encoder Successfully Started at value = %d\n", prevEncoderValue);
  encoder1->attachCallback(callBack);

  for (;;) {
    xQueueReceive(encoderQueue, &currentEncoderstate, portMAX_DELAY);
    printf("Encoder: ");
    currentValue = currentEncoderstate.currentValue;
    if (currentValue != prevEncoderValue) {
      printf("%d\n", currentValue);
      prevEncoderValue = currentValue;
    } else {
      switch (currentEncoderstate.currentClick) {
        case NewEncoder::UpClick:
          printf("at upper limit.\n");
          break;

        case NewEncoder::DownClick:
          printf("at lower limit.\n");
          break;

        default:
          break;
      }
    }
    //printf("High watermark: %d\n", uxTaskGetStackHighWaterMark( NULL));
  }
  vTaskDelete(nullptr);
}

void ESP_ISR callBack(NewEncoder*encPtr, const volatile NewEncoder::EncoderState *state, void *uPtr) {
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

  xQueueOverwriteFromISR(encoderQueue, (void * )state, &pxHigherPriorityTaskWoken);
  if (pxHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}
