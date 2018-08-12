# NewEncoder
Interrupt-driven rotary encoder library. Specifically designed for encoders that complete a fully quadrature cycle between successive detents. For example, [this one from Adafruit](https://www.adafruit.com/product/377). Some other libraries record multiple steps per detent with this type of encoder.

Two interrupt-capable pins are required for each encoder connected. Thus, only one encoder can be used with an Arduino Uno for example.

The encoder's "A" and "B" terminals should be connected to the processor's inputs and its common terminal should be grounded. The library enables the processor's internal pull-ups, so external ones are not required.
## Library Description:
#### Class NewEncoder
This provides interfacing to the encoder, interrupt handling, and rotation counting. One instance of this class is created for each encoder.
#### Public NewEncoder Members Functions:
***Constructor - creates  and configures object***

    NewEncoder(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue)
**Arguments:**
 - **uint8_t aPin** - Hardware pin connected to the encoder's "A" terminal.
 - **uint8_t bPin** - Hardware pin connected to the encoder's "B" terminal.
 - **int16_t minValue** - Lowest count value to be returned. Further anti-clockwise rotation produces no further change in output.
 - **int16_t maxValue** - Highest count value to be returned. Further clockwise rotation produces no further change in output.
 - **int16_t initalValue** - Initial encoder value. Should be between minValue and maxValue
 
 ***Constructor - only creates object*** 

    NewEncoder()
**Arguments:** None. The object must be configured using the configure() method before being used

 ***Configure or Re-configure an encoder object*** 

    void configure(uint8_t aPin, uint8_t bPin, int16_t minValue, int16_t maxValue, int16_t initalValue);
**Arguments:** Same as Constructor

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
   **Returns:**    Value actually, as int16_t (may be ignored)
   Note: The library overrides the ***assignment operator***. So, if ***myEncoder*** is an encoder object, the following two statements are equivalent:
   
    myEncoder.setValue(x);
    myEncoder = x;

