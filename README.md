# NewEncoder
Interrupt-driven rotary encoder library. Works both with encoders that produce one complete quadrature cycle every detent (such as [this Bourns unit sold by Adafruit](https://www.adafruit.com/product/377)) and those that produce one complete quadrature cycle for every two detents (such as [this Alps unit sold by Mouser](https://www.mouser.com/ProductDetail/alps/ec11e15244g1/?qs=YMSFtX0bdJDiV4LBO61anw==&countrycode=US&currencycode=USD)).

The encoders' switches are debounced using a state table approach.

Two interrupt-capable pins are required for each encoder connected. Thus, only one encoder can be used with an Arduino Uno for example.

The encoders' "A" and "B" terminals should be connected to the processor's inputs and its common terminal should be grounded. The library enables the processor's internal pull-ups, so external ones are not required.
## Library Description:
#### Class NewEncoder
This provides interfacing to the encoder, interrupt handling, and rotation counting. One instance of this class is created for each encoder.
#### Public NewEncoder Members Functions:
***Constructor - creates  and configures object***

    NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE)
**Arguments:**
 - **uint8_t aPin** - Hardware pin connected to the encoder's "A" terminal.
 - **uint8_t bPin** - Hardware pin connected to the encoder's "B" terminal.
 - **int16_t minValue** - Lowest count value to be returned. Further anti-clockwise rotation produces no further change in output.
 - **int16_t maxValue** - Highest count value to be returned. Further clockwise rotation produces no further change in output.
 - **int16_t initalValue** - Initial encoder value. Should be between minValue and maxValue
 - **uint8_t type** Type of encoder - FULL_PULSE (default, one quadrature pulse per detent) or HALF_PULSE (one quadrature pulse for every two detents)
 
 ***Constructor - only creates object*** 

    NewEncoder()
**Arguments:** None. The object must be configured using the configure() method before being used

 ***Configure or Re-configure an encoder object*** 

    void configure(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue, uint8_t type = FULL_PULSE);
**Arguments:** Same as Constructor
**Returns:**    Nothing

 ***Start an encoder object*** 
 
    bool begin();
    
   **Arguments:** None
   
   **Returns:**
    - true if encoder object successfully started, false otherwise

 ***Disable an encoder object*** 

     void end();
  **Arguments:** None
   
  **Returns:**    Nothing
   
 ***Check if encoder object is enabled*** 

     bool enabled();
  **Arguments:** None
  
  **Returns:**
    - true if encoder object is enabled, false otherwise
 
   ***Get current encoder value*** 
   
     int16_t getValue();
  **Arguments:** None
  
  **Returns:**    current encoder value, as int16_t
   
   Note: The library overrides ***operator int16_t***. So, if ***myEncoder*** is an encoder object, the following two statements are equivalent:
   

    x = myEncoder.getValue();
    x = myEncoder;
 
   ***Set current encoder value*** 
   
     int16_t setValue(int16_t val);
  **Arguments:**
   - **int16_t val** - New encoder value. If required, it is constrained to be between **minValue** and **maxValue**.
  
  **Returns:**    Value actually set, as int16_t (may be ignored)
   
   Note: The library overrides the ***assignment operator***. So, if ***myEncoder*** is an encoder object, the following two statements are equivalent:
   
    myEncoder.setValue(x);
    myEncoder = x;

   ***Get current encoder value and set to new value***
   
    int16_t getAndSet(int16_t val);
**Arguments:**
   - **int16_t val** - New encoder value. If required, it is constrained to be between **minValue** and **maxValue**.
  
   **Returns:**    Value of encoder before being set. 
   
   Note: This function reads the current encoder value and then sets the new value in an atomic manner. No counts will be missed as an interrupt can not occur between the  reading and setting. Useful for periodically reading encoder attached to spinning motor to determine rotation speed.
  
   ***Check if encoder has been rotated*** 
   
    bool upClick();
    bool downClick();
  **Arguments:** None
  
  **Returns:**  One of these methods will return true if the encoder has been rotated at least one detent in the associated directon (upClick() for CW, downClick() for CCW). False otherwise.
  
   Note: Each call clears its internal flag. So, function will not return true again until another full-detent rotation has ocurred. Also, the functions will return true even if the encoder's value is saturated at the lower or upper limit.
    
## Credits:
The **direct_pin_read.h** and **interrupt_pins.h** header files were "borrowed" directly from the [PRJC Encoder Library](https://www.pjrc.com/teensy/td_libs_Encoder.html) Copyright (c)  PJRC.COM, LLC - Paul Stoffregen. All typical license verbiage applies.

Other concepts were inspired by http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
