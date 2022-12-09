// interrupt pins for known boards

// Teensy (and maybe others) define these automatically
#if !defined(CORE_NUM_INTERRUPT)

// Wiring boards
#if defined(WIRING)
  #define CORE_NUM_INTERRUPT	NUM_EXTERNAL_INTERRUPTS


// Arduino Uno, Duemilanove, Diecimila, LilyPad, Mini, Fio, etc...
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) ||defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__)
  #define CORE_NUM_INTERRUPT	2

// Arduino Mega
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  #define CORE_NUM_INTERRUPT	6

// Arduino Nano Every, Uno R2 Wifi
#elif defined(__AVR_ATmega4809__)
  #define CORE_NUM_INTERRUPT	13

// Arduino Leonardo (untested)
#elif defined(__AVR_ATmega32U4__) && !defined(CORE_TEENSY)
  #define CORE_NUM_INTERRUPT	5

// Sanguino (untested) and ATmega1284P
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega1284P__)
  #define CORE_NUM_INTERRUPT	3

// ATmega32u2 and ATmega32u16 based boards with HoodLoader2
#elif defined(__AVR_ATmega32U2__) || defined(__AVR_ATmega16U2__)
  #define CORE_NUM_INTERRUPT 8

// Chipkit Uno32 - attachInterrupt may not support CHANGE option
#elif defined(__PIC32MX__) && defined(_BOARD_UNO_)
  #define CORE_NUM_INTERRUPT	5

// Chipkit Uno32 - attachInterrupt may not support CHANGE option
#elif defined(__PIC32MX__) && defined(_BOARD_MEGA_)
  #define CORE_NUM_INTERRUPT	5

// http://hlt.media.mit.edu/?p=1229
#elif defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  #define CORE_NUM_INTERRUPT    1

// ATtiny441 ATtiny841
#elif defined(__AVR_ATtiny441__) || defined(__AVR_ATtiny841__)
  #define CORE_NUM_INTERRUPT	1

//https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/ATtiny_x313.md
#elif defined(__AVR_ATtinyX313__)
  #define CORE_NUM_INTERRUPT    2
 
// Attiny167 same core as abobe
#elif defined(__AVR_ATtiny167__)
  #define CORE_NUM_INTERRUPT	2
  
// Arduino Due
#elif defined(__SAM3X8E__) 
  #define CORE_NUM_INTERRUPT	54

// ESP8266 (https://github.com/esp8266/Arduino/)
#elif defined(ESP8266)
  #define CORE_NUM_INTERRUPT EXTERNAL_NUM_INTERRUPTS

// ESP32 (https://github.com/espressif/arduino-esp32)
#elif defined(ESP32)

  #define CORE_NUM_INTERRUPT  40 

// Arduino Zero - TODO: interrupts do not seem to work
//                      please help, contribute a fix!
#elif defined(__SAMD21G18A__)
  #define CORE_NUM_INTERRUPT	31

#elif defined(__SAMD51__)
  #define CORE_NUM_INTERRUPT	26

// Arduino 101
#elif defined(__arc__)
  #define CORE_NUM_INTERRUPT	14

// STM32 ... UNTESTED!!!
#elif ARDUINO_ARCH_STM32
#define CORE_NUM_INTERRUPT NUM_DIGITAL_PINS
#endif
#endif

#if !defined(CORE_NUM_INTERRUPT)
#error "Interrupts are unknown for this board. Please specify in interrupt_pins.h"
#else
static_assert(CORE_NUM_INTERRUPT > 1, "NewEncoder requires at least 2 interrupt pins. The currently-selected board does not meet this requirement.");
#endif



