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
    int b = (i + 3) / 13.0 * 255;      // Calculate brightness of the color based on the displayed digit
    numeral.writeDigit(0, i, b/3, b, b/2);  // Use ratios of each color to produce a minty green that gets brighter without changing hue
    delay(150);
  }

  for (int i = 8; i > 0; i--) {
    int b = (i + 3) / 13.0 * 255;
    numeral.writeDigit(0, i, b/3, b, b/2);
    delay(150);
  }
}
