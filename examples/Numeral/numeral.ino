/* # Clueboard Numeral Library
 *  
 * Support for using the Clueboard Numeral (i2c RGB 7 Segment display) from Arduino
 * 
 * # Example Sketch: 

void setup() {
  // Optional: Add Numerals manually. If you do this 
  // the i2c bus will not be scanned, and only the 
  // Numerals you specify will be used.
  //numeralAddDevice(0x73);
  //numeralAddDevice(0x72);
  //numeralAddDevice(0x71);
  //numeralAddDevice(0x70);
  
  // Setup the connected numerals so they're ready to use
  numeralSetup();
}

void loop() {
  // Count from 0 to 9 in a nice minty green, getting brighter with higher numbers.
  for (int i = 0; i < 10; i++) {
    int brightness = (i + 3) / 13.0 * 255; // Calculate brightness of the color based on the displayed digit
    int r = b/3;                    // Set red to always be 1/3rd of green's brightness
    int g = b;                      // Set green to our current brightness level
    int b = b/2;                    // Set blue to half of green's brightness
    numeralDisplay(0, i, r, g, b);  // Display the current digit
    delay(150);                     // Wait 150 ms before displaying the next digit
  }
}

 * # Glossary
 *   The following variables are used frequently in this library.
 *   
 * `int digit`
 *    A digit that is displayed on a numeral.
 *    
 * `int numeral`
 *     Used to distinguish up to 4 Numerals connected to the same 
 *     i2c bus. Numerals are auto-detected and are typicaly 
 *     indexed in descending order, IE `{0x73, 0x72, 0x71, 0x70} when 
 *     all 4 are connected.
 *     
 * `int segment`
 *     Labeled `A, B, C, D, E, F, G, and DP, this is used with `segmentMap` 
 *     to determine which LEDs are associated with each segment.
 * 
 * # Setup
 * 
 * In most cases setup is very easy, just add `numeralSetup()` to your sketch's
 * `setup()` function. This will scan the i2c bus and add all the Numerals that
 * are detected, then initialize them.
 * 
 * `void numeralSetup()`
 *   Setup our numerals so they're ready to use.
 * 
 * `void numeralAddDevice(int address)`
 *   Add a numeral to our list of known devices. Normally you do not
 *   need to use this. You should only use this if the auto-detection
 *   code does not work in the way that you need.
 * 
 * # High Level Interface (API)
 * 
 * For most applications you will want to use this function.
 * 
 * `void numeralDisplay(int numeral, int digit, int red, int green, int blue)`
 *   Display a digit (0-9) on a numeral.
 *  
 *   `numeral`
 *      The numeral to display this digit on
 *      
 *   `address`
 *     The i2c address of the numeral, in the range 0x70-0x74.
 * 
 * # Mid-level Interface (API)
 * 
 * This provides direct access to manipulate the color of each individual segment.
 * Gamma correction is performed at this level for more linear color representation.
 * 
 * `void numeralSegment(int numeral, int segment, int r, int g, int b)`
 *   Set the color for an individual segment. Channel values will be 
 *   gamma corrected for a more linear brightness response.
 *   
 *   Physical Layout:
 *  
 *       |-A-|
 *       F   B
 *       |-G-|
 *       E   C
 *       |-D-| DP
 *  
 *   `segment`
 *     The segment to set a color for. Use one of the segment<L> 
 *     variables, such as `segmentA`, `segmentG`, or `segmentDP`.
 * 
 *   `r`
 *     Brightness for the Red channel (0-255)
 *
 *   `g`
 *     Brightness for the Green channel (0-255)
 *
 *   `b`
 *     Brightness for the Blue channel (0-255)
 *
 * # Low-level Interface (API)
 * 
 * Internally this library stores the PWM value of each LED in the
 * `pwmState[][]` array. The outer array consists of one entry per
 * numeral, while the inner array is the PWM value (0-255) for
 * each LED on the is31fl3235a IC. You can manipluate these bytes
 * and then call `is31fl3235aUpdatePWM()` to write them out to
 * the IC(s) efficiently.
 * 
 * There is a similar array that stores the on/off state for each
 * LED called `ledState[][]`, with a similar structure. However, the
 * values for the inner array are all boolean. After updating the
 * values call `is31fl3235aUpdateLED()` to write them to the IC(s).
 */

#include <Wire.h>

/* Registers used by the is31fl3235a
 */
const int ledRegisterStart = 0x2A;       // on/off status for OUT1. The next 27 registers control OUT2-OUT28. (Values: 0-1)
const int ledRegisterEnd = 0x45;         // on/off status for OUT28. (Values: 0-1)
const int pwmRegisterStart = 0x05;        // Brightness control for OUT1. The next 27 registers control brightness for OUT2-OUT28. (Values: 0-255)
const int pwmRegisterEnd = 0x20;         // Brightness control for OUT28. (Values: 0-255)
const int pwmRegisterUpdate = 0x25;      // Write 0 to update live PWM values with register values (Values: 0)
const int shutdownRegister = 0x00;       // 0 = software shutdown, 1 = normal operation (Values: 0/1)
const int globalControlRegister = 0x4A;  // 0 = normal operation, 1 = shutdown all LEDs (Values: 0/1)
const int pwmFrequencyRegister = 0x4B;   // Set PWM frequency, 0 = 3kHz, 1 = 22kHz (Values: 0/1)
const int resetRegister = 0x4F;          // Write 0 to reset all registers to their default value (Values: 0)


/* LED to numeral segment map
 *  
 * Outer arrays are the segments in order.
 * 
 * Inner array is the index of the Red, Green, and Blue LEDs.
 */
const int PROGMEM segmentMap[8][3] = {
// R   G   B
  {17, 16, 15},  // A
  {22, 21, 20},  // B
  {26, 27, 28},  // C
  {1,  2,  3},   // D
  {4,  5,  6},   // E
  {9,  7,  8},   // F
  {14, 13, 12},  // G
  {23, 24, 25}   // DP
};


/* Map of segment letters to segmentMap entries.
 */
const int segmentA = 0;
const int segmentB = 1;
const int segmentC = 2;
const int segmentD = 3;
const int segmentE = 4;
const int segmentF = 5;
const int segmentG = 6;
const int segmentDP = 7;


/* Map of digits to segments.
 *  
 * Each inner array is a list of booleans that correspond to segments a-f.
 */
const int PROGMEM digitSegments[11][7] {
// segments
// a  b  c  d  e  f  g      digits
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}, // 9
  {0, 0, 0, 0, 0, 0, 0}  // blank
};


/* Track the Numerals connected to our i2c bus
 */
int foundNumerals = 0;
int devAddress[4];


/* The on/off state for each LED. Sent directly to the is31fl3235a using the register incrementing feature.
 */
extern int ledState[4][29] {
  {ledRegisterStart,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {ledRegisterStart,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {ledRegisterStart,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {ledRegisterStart,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};


/* The current brightness for each LED. Sent directly to the is31fl3235a using the register incrementing feature.
 */
extern int pwmState[4][29] {
  {pwmRegisterStart,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {pwmRegisterStart,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {pwmRegisterStart,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {pwmRegisterStart,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};


/* Gamma correction table to make the LED brightness scale more linearly.
 */
const int PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255
};


void numeralDisplay(int numeral, int digit, int r, int g, int b) {
  /* Display a digit (0-9) on a numeral.
   *  
   * `numeral`
   *    The numeral to display this digit on
   */
  for (int segment = segmentA; segment <= segmentG; segment++) {
    if (pgm_read_byte(&digitSegments[digit][segment])) {
      numeralSegment(numeral, segment, r, g, b);
    } else {
      numeralSegment(numeral, segment, 0, 0, 0);
    }
  }
  
  is31fl3235aUpdatePWM(numeral);
}


void numeralSegment(int numeral, int segment, int r, int g, int b) {
  /* Set the color for an individual segment. Channel values will be 
   *  gamma corrected for a more linear brightness response.
   *  
   * `segment`
   *     The segment to set a color for. Use one of the segment<L> 
   *     variables above, such as `segmentA`.
   * 
   * `r`
   *     Brightness for the Red channel (0-255)
   *
   * `g`
   *     Brightness for the Green channel (0-255)
   *
   * `b`
   *     Brightness for the Blue channel (0-255)
   */
  int pwmR = pgm_read_byte(&segmentMap[segment][0]);
  int pwmG = pgm_read_byte(&segmentMap[segment][1]);
  int pwmB = pgm_read_byte(&segmentMap[segment][2]);

  pwmState[numeral][pwmR] = pgm_read_byte(&gamma8[r]);
  pwmState[numeral][pwmG] = pgm_read_byte(&gamma8[g]);
  pwmState[numeral][pwmB] = pgm_read_byte(&gamma8[b]);
}


void is31fl3235aUpdateLED(int numeral) {
  /* Update the live LED on/off values
   */
  i2cWrite(devAddress[numeral], ledState[numeral], sizeof(ledState[numeral]));    
}


void is31fl3235aUpdatePWM(int numeral) {
  /* Write the pending LED changes to the numeral.
   */
  i2cWrite(devAddress[numeral], pwmState[numeral], sizeof(pwmState[numeral]));    
  writeRegister(numeral, pwmRegisterUpdate, 0x00);
}


void numeralSetup() {
  /* Setup our numerals so they're ready to use.
   */
  Wire.begin();

  /* If the end user wants to specify the order for their numerals they can call 
   * `numeralAddDevice()` directly. Otherwise we scan the i2c bus for anything 
   * responding on the numeral addresses. 
   */
  if (devAddress[0] == 0) {
    i2cScan();
  }

  // Iterate through our found numerals and initialize each one.
  for (int numeral = 0; numeral < foundNumerals; numeral++) {
    is31fl3235aReset(numeral);            // Set IC to default state
    is31fl3235aUpdatePWM(numeral);        // Set all PWM to 0 (off)
    is31fl3235aUpdateLED(numeral);        // Set all LED to on
    is31fl3235aPWMFrequency(numeral, 1);  // Set pwm frequency to 22kHz
    is31fl3235aICPower(numeral, 1);       // Begin normal operation
  }
}


void is31fl3235aReset(int numeral) {
  /* Reset the IC to default settings
   */
  writeRegister(numeral, resetRegister, 0x00);
}


void is31fl3235aICPower(int numeral, bool shutdownState) {
  /* Turns the IC on and off for power savings
   *  
   * `numeral`
   *     The numeral to set
   *     
   * `shutdownState`
   *     Whether the IC should be on (1) or off (0)
   */
  writeRegister(numeral, shutdownRegister, shutdownState);
}


void is31fl3235aPWMFrequency(int numeral, int frequency) {
  /* Set the ic's PWM frequency.
   *  
   * `frequency`
   *     Set the frequency to 3kHz (0) or 22kHz (1)
   */
  writeRegister(numeral, pwmFrequencyRegister, frequency);
}


void writeRegister(int numeral, int ICRegister, int data) {
  /* Write a single byte to a register on the IC.
   *  
   *  `int numeral`
   *      The numeral to write to (0-3)
   *
   *  `int ICRegister`
   *      The register on the is31fl3235a to write
   *
   *  `int data`
   *      The single byte to write to this register
   */
  int cmd[2];

  cmd[0] = ICRegister;
  cmd[1] = data;

  if (devAddress[numeral] != 0) {
    i2cWrite(devAddress[numeral], cmd, sizeof(cmd));
  }
}


void i2cWrite(int ICAddress, int *data, int num) {
  /* Low level function to write data to the i2c bus.
   *  
   *  `ICAddress`
   *      The address of the ic we're writing to
   *
   *  `data`
   *      A pointer to the data to write to the slave
   *
   *  `num`
   *      The length of data
   */
  Wire.beginTransmission(ICAddress);
  for (int i=0;i<num;i++) {
    Wire.write(*(data+i));
  }
  Wire.endTransmission();
}

void numeralAddDevice(int address) {
  /* Add a numeral to our list of known devices.
   *  
   * `address`
   *     The i2c address of the numeral, in the range 0x70-0x74.
   */
  devAddress[foundNumerals] = address;
  foundNumerals++;
}

void i2cScan() {
  /* Scan the i2c looking for numerals. 
   *  
   * There are only 4 addresses to check and we don't verify that the device that responds is an is31fl3235a. It's possible we may cause problems if other devices are on one of the 4 addresses we can use.
   */
  for (int address = 0x3f; address > 0x3b; address--) {
    Wire.beginTransmission(address);
    
    if (Wire.endTransmission() == 0) {
      // We got an answer from a device on this address, add it to the list.
      numeralAddDevice(address);
    }
  }
}


/* User code goes below this comment
 */
void setup() {
  // put your setup code here, to run once:
  
  numeralSetup();
}


void loop() {
  // put your main code here, to run repeatedly:

  /* Count from 0 to 9 to 0 in a nice minty green, getting brighter with higher numbers.
   */
  for (int i = 0; i < 10; i++) {
    int b = (i + 3) / 13.0 * 255;      // Calculate brightness of the color based on the displayed digit
    numeralDisplay(0, i, b/3, b, b/2);  // Use ratios of each color to produce a minty green that gets brighter without changing hue
    delay(150);
  }
  for (int i = 8; i > 0; i--) {
    int b = (i + 3) / 13.0 * 255;
    numeralDisplay(0, i, b/3, b, b/2);
    delay(150);
  }
}
