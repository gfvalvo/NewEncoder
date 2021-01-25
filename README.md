# NewEncoder
Interrupt-driven rotary encoder library. Works both with encoders that produce one complete quadrature cycle every detent (such as [this Bourns unit sold by Adafruit](https://www.adafruit.com/product/377)) and those that produce one complete quadrature cycle for every two detents (such as [this Alps unit sold by Mouser](https://www.mouser.com/ProductDetail/alps/ec11e15244g1/?qs=YMSFtX0bdJDiV4LBO61anw==&countrycode=US&currencycode=USD)).

The encoders' switches are debounced using a state table approach.

Two interrupt-capable pins are required for each encoder connected. Thus, only one encoder can be used with an Arduino Uno for example.

The encoders' "A" and "B" terminals should be connected to the processor's inputs and its common terminal should be grounded. The library enables the processor's internal pull-ups, so external ones are not required.
# Version 2.x:
This is a major update to the NewEncoder library. It addresses discrepancies that could occur due to the nature of interrupts. Previous version could provide inconsistent results between the getValue(), upClick(), and downClick() functions.

For example a call to getValue() might indicate that the encoder was not moved, but a call to upClick() or downClick() immediately following this could show that it was. Or, upClick() might have indicated no change from the encoder but a call to getValue() immediately following this could return an incremented value.

These non-intuitive results depended on the exact timing of the calls to the various functions in relation to the exact moment the encoder caused the processor interrupt.

This new version of the library addresses these issues by introducing new member functions for accessing the encoder and new data structures that will be updated atomically. The older member functions are still available. However, they have been deprecated and will cause compiler warnings if used. They may be removed from a future release of this library.
### New Datatypes in Version 2.0:
The following public datatypes are in NewEncoder scope (i.e. `NewEncoder::`).

    enum EncoderClick {
		NoClick, DownClick, UpClick
	};
This enum datatype indicates the direction of encoder movement.

    struct EncoderState {
		int16_t currentValue = 0;
		EncoderClick currentClick = NoClick;
	};
This struct datatype contains the current encoder value and click direction. Variables of these types are returned by several of the member functions new to Version 2.0. Those functions are described in the next section.
#### All examples have been updated to use the new datatypes / functions described.

**NOTES:**

**1. This library is interrupt-safe for the single-core / single-thread platforms that make up the majority of the Arduino Ecosystem. It is also safe for the FreeRTOS / ESP32 platform if its functions are only called from a single core and single thread (task). The SingleEncoderEsp32FreeRTOS example follows these guidelines. A NewEncoderEsp32 library will soon be released that is multi-thread / multi-core safe.**

**2. If desired, the previous version of this library can be downloaded and used. However, it will no longer be supported / updated: [NewEncoder v1.4](https://github.com/gfvalvo/NewEncoder/releases/tag/v1.4)**

# Library Description:
## Class NewEncoder
This provides interfacing to the encoder, interrupt handling, and rotation counting. One instance of this class is created for each encoder.
## Public NewEncoder Members Functions:
### Constructor - creates  and configures object

    NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE)
**Arguments:**
 - **uint8_t aPin** - Hardware pin connected to the encoder's "A" terminal.
 - **uint8_t bPin** - Hardware pin connected to the encoder's "B" terminal.
 - **int16_t minValue** - Lowest count value to be returned. Further anti-clockwise rotation produces no further change in output.
 - **int16_t maxValue** - Highest count value to be returned. Further clockwise rotation produces no further change in output.
 - **int16_t initalValue** - Initial encoder value. Should be between minValue and maxValue
 - **uint8_t type** Type of encoder - FULL_PULSE (default, one quadrature pulse per detent) or HALF_PULSE (one quadrature pulse for every two detents)
 
### Constructor - only creates object

    NewEncoder()
**Arguments:** None. The object must be configured using the configure() method before being used

### Configure or Re-configure an encoder object

    void configure(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE);
**Arguments:** Same as Constructor
**Returns:**    Nothing

### Start an encoder object
 
    bool begin();
    
   **Arguments:** None
   
   **Returns:**
    - true if encoder object successfully started, false otherwise

### Disable an encoder object

     void end();
  **Arguments:** None
   
  **Returns:**    Nothing
  
### Check if encoder object is enabled

     bool enabled();
****Arguments:**** None
  
****Returns:****
    - `true` if encoder object is enabled, `false` otherwise
 
### Get Current Encoder State 
    bool getState(NewEncoder::EncoderState &state);
****Arguments:****
- **NewEncoder::EncoderState &state** - Reference to an `EncoderState` object. The current state of the encoder will be written into this object.

****Returns:****
      - `true` if the state of the encoder has changed since the last call to this function. `false` otherwise.
 
 ### Get Current Encoder State and Change Value
    bool getAndSet(int16_t val, NewEncoder::EncoderState &oldState, NewEncoder::EncoderState &newState);
 ****Arguments:****
 - **int16_t val** - The new encoder value. The function may change this value to bring it between `minValue` and `maxValue`.
 - **NewEncoder::EncoderState &oldState** - Reference to an `EncoderState` object. The state of the encoder **before** its value was changed will be written into this object.
 - **NewEncoder::EncoderState &newState** - Reference to an `EncoderState` object. The state of the encoder **after** its value was changed will be written into this object.

****Returns:****
      - `true` if the value returned in `Oldstate` represents an unread encoder state change. `false` otherwise.      
 
 ### Get Change Encoder Settings and Get the New State
    bool newSettings(int16_t newMin, int16_t newMax, int16_t newCurrent, EncoderState &state);
 ****Arguments:****
 - **int16_t newMin** - new `minVal`.
 - **int16_t newMax** - new `maxVal`.
 - **int16_t newCurrent** - new encoder value.
 - **NewEncoder::EncoderState &state** - Reference to an `EncoderState` object. The state of the encoder **after** its value was changed will be written into this object.

****Returns:****
      - `true` if newMin < newMax. `false` otherwise. 
  
 ### Attach Callback function to be invoked when encoder is rotated
    void attachCallback(void (*EncoderCallBack)(NewEncoder *, const volatile NewEncoder::Encoder State *, void *),  void *uPtr = nullptr);
 ****Arguments:****
 - **void (\*EncoderCallBack)(NewEncoder \*, const volatile NewEncoder::EncoderState \*, void \*)** - Pointer to the callback function. This function returns no value and must accept arguments of
     - Pointer to an NewEncoder object
     - Pointer to a const volatile NewEncoder::EncoderState
     - Pointer to a void
 - **void \*uPtr = nullptr** - A user-defined pointer that will be supplied as an argument to the callback function.

      
 ****Returns:**** Nothing
 The callback function will be invoke anytime the encoder is rotated. Its argument will be a pointer to the encoder object itself, a pointer to the encoder's current state, and the void \* that was supplied when attachCallback() was called. Note: The callback function is called from an ISR. So, it must use ISR-safe coding techniques. See the 'SingleEncoderWithCallback' example

### Customize increment/decrement and min/max behavior via inheritance

    // This function may be implemented in an inherited class to customize the increment/decrement and min/max behavior.
    // See the source code and CustomEncoder example
    // Caution - this function is called in interrupt context.
    // See "CustomEncoder" example
    virtual void updateValue(uint8_t updatedState);
    
 ****Arguments:****
 - **int8_t updatedState** - New state vector of the encoder that includes the INCREMENT_DELTA and DECREMENT_DELTA bits
 
 ****Returns:**** Nothing
 
 # DEPRECATED FUNCTIONS - THESE MAY BE DELETED FROM FUTURE RELEASES:
  ***Get current encoder value - DEPRECATED***
   
     int16_t getValue();
  **Arguments:** None
  
  **Returns:**    current encoder value, as int16_t
   
   Note: The library overrides ***operator int16_t***. So, if ***myEncoder*** is an encoder object, the following two statements are equivalent:
   

    x = myEncoder.getValue();
    x = myEncoder;
 
   ***Set current encoder value - DEPRECATED*** 
   
     int16_t setValue(int16_t val);
  **Arguments:**
   - **int16_t val** - New encoder value. If required, it is constrained to be between **minValue** and **maxValue**.
  
  **Returns:**    Value actually set, as int16_t (may be ignored)
   
   Note: The library overrides the ***assignment operator***. So, if ***myEncoder*** is an encoder object, the following two statements are equivalent:
   
    myEncoder.setValue(x);
    myEncoder = x;

   ***Get current encoder value and set to new value - DEPRECATED***
   
    int16_t getAndSet(int16_t val);
**Arguments:**
   - **int16_t val** - New encoder value. If required, it is constrained to be between **minValue** and **maxValue**.
  
   **Returns:**    Value of encoder before being set. 
   
   Note: This function reads the current encoder value and then sets the new value in an atomic manner. No counts will be missed as an interrupt can not occur between the  reading and setting. Useful for periodically reading encoder attached to spinning motor to determine rotation speed.
  
   ***Check if encoder has been rotated - DEPRECATED*** 
   
    bool upClick();
    bool downClick();
  **Arguments:** None
  
  **Returns:**  One of these methods will return true if the encoder has been rotated at least one detent in the associated directon (upClick() for CW, downClick() for CCW). False otherwise.
  
   Note: Each call clears its internal flag. So, function will not return true again until another full-detent rotation has ocurred. Also, the functions will return true even if the encoder's value is saturated at the lower or upper limit.

   ***Change min, max, and current value - DEPRECATED***
   
    bool newSettings(int16_t newMin, int16_t newMax, int16_t newCurrent);
**Arguments:**
   - **int16_t newMin** - New Minimum setting
   - **int16_t newMax** - New Maximum setting
   - **int16_t newCurrent** - New Encoder value
   
**Returns:**  - true if change was successful, false otherwise
    
## Credits:
The **direct_pin_read.h** and **interrupt_pins.h** header files were "borrowed" directly from the [PRJC Encoder Library](https://www.pjrc.com/teensy/td_libs_Encoder.html) Copyright (c)  PJRC.COM, LLC - Paul Stoffregen. All typical license verbiage applies.

Other concepts were inspired by http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
