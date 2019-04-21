# Clueboard Numeral Arduino Library

The Clueboard Numeral is an RGB 7 segment display. Each segment is capable of displaying up to 16,581,375 different colors by combining Red, Green, and Blue channels.

# Setup

In most cases setup is very easy, just add `numeral.setup()` to your sketch's
`setup()` function. This will scan the i2c bus and add all the Numerals that
are detected, then initialize them.

If for some reason the auto-detection doesn't work for you, or you want a different order for your Numerals, you can call `numeral.addNumeral(address);` one or more times to configure them manually.

# High Level API

There are two ways most people will use Numeral. You can treat each connected numeral as an individual display using `writeDigit(int numeral, int digit, int r, int g, int b)`. You can also treat all connected numerals as a single display that will display up to 4 digits using `writeNumber(int number, int r, int g, int b)`.

## `numeral.writeDigit()`

`void writeDigit(int numeral, int digit, int r, int g, int b)`

This function can be used to write a single digit to an individual numeral.

    numeral
        The index of the numeral to write to (0-3)

    digit
        The digit to display (0-9)

    r
        How bright the red channel should be (0-255)

    g
        How bright the blue channel should be (0-255)

    b
        How bright the green channel should be (0-255)

## `numeral.writeNumber()`

`void writeNumber(int number, int r, int g, int b)`

This function can be used to write up to 4 digits to connected numerals.

    number
        The base-10 number to display. This will be truncated to the last 4 digits if larger than 9999. (0-9999)

    r
        How bright the red channel should be (0-255)

    g
        How bright the blue channel should be (0-255)

    b
        How bright the green channel should be (0-255)

## Example Sketch

```c++
#include <Numeral.h>

Numeral numeral;

void setup() {
  // put your setup code here, to run once:

  numeral.setup();
}

void loop() {
  // put your main code here, to run repeatedly:

  /* Count from 0 to 9 to 0 in a nice minty green, getting brighter with higher numbers.
   */
  for (int i = 0; i < 10; i++) {
    int b = (i + 3) / 13.0 * 255;           // Calculate brightness of the color based on the displayed digit
    numeral.writeDigit(0, i, b/3, b, b/2);  // Use ratios of each color to produce a minty green that gets brighter without changing hue
    delay(150);
  }

  for (int i = 8; i > 0; i--) {
    int b = (i + 3) / 13.0 * 255;
    numeral.writeDigit(0, i, b/3, b, b/2);
    delay(150);
  }
}
```

# Mid-level Interface

For more control over color you can exert direct control over each segment on the display. The display will not be updated immediately, you will have to manually update the display after making changes.

`void segment(int numeral, int segment, int r, int g, int b)`

Use this function to directly control individual segments. The segments are labled 0-7 and control individual letter segments plus the dot. There are variables, `segmentA` through `segmentG` plus `segmentDP` that map to these segments.

**Segment Layout:**

```
    |-A-|
    F   B
    |-G-|
    E   C
    |-D-| DP
```

`void update(void);`

Use this function to display your pending changes.

# Low Level Interface

If you'd like more direct control over the display on the numeral you can manipulate the LED and PWM registers directly.

`const int PROGMEM segmentMap[8][3]`

This is a map of segments to LED locations. The outer array maps to the segment layout above. The inner array is a triplet of Red, Green, and Blue LED indexes. You can use this to determine which LEDs correlate to particular segments.

`int ledState[3][29]`

This is an array of arrays that controls the on/off status of each individual LED. The first element of the array is a register address that must not be modified. The rest of the array is a boolean value that indicates if the corresponding LED should be on or off.

`int pwmState[4][29]`

This is an array of arrays that controls the PWM value of each individual LED. The first element of the array is a register address that must not be modified. The rest of the array is an integer value (0-255) that sets the brightness for that LED.

`void update(void);`

Use this function to display your pending changes.
