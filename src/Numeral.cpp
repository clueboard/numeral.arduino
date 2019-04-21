/*
 * Numeral.c - Arduino library for interfacing with the Clueboard Numeral devices.
 * Created by Zach White, 2019 Apr 18
 * Copyright 2019 Clueboard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Arduino.h"
#include "Numeral.h"
#include <Wire.h>

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

void Numeral::setup(void) {
    Wire.begin();
    if (count == 0) {
        i2cScan();
    }

    // Iterate through our found numerals and initialize each one.
    for (int numeral = 0; numeral < count; numeral++) {
        _writeRegister(numeral, resetRegister, 0x00);      // Set IC to default state
        _updatePWM(numeral);                               // Set all PWM to 0 (off)
        _updateLED(numeral);                               // Set all LED to on
        _writeRegister(numeral, pwmFrequencyRegister, 1);  // Set pwm freq to 22kHz
        power(numeral, 1);                                 // Begin normal operation
    }
}

void Numeral::addNumeral(int i2cAddress) {
    /* Add a numeral to the list of known numeral devices.
     */
    i2cAddresses[count] = i2cAddress;
    count++;
}

void Numeral::i2cScan(void) {
    /* Scan the i2c bus looking for numerals. 
     *  
     * There are only 4 addresses to check and we don't verify that the device
     * that responds is an is31fl3235a. It's possible we may cause problems if
     * other devices on the bus are using one of the 4 addresses we can use. In
     * that event users will need to pass addresses to Numeral::Numeral().
     */
    for (int address = numeralAddressEnd; address > numeralAddressStart; address--) {
        Wire.beginTransmission(address);

        if (Wire.endTransmission() == 0) {
            addNumeral(address);
        }
    }
}

void Numeral::_writeRegister(int numeral, int ICRegister, int data) {
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

    if (i2cAddresses[numeral] != 0) {
        _i2cWrite(i2cAddresses[numeral], cmd, sizeof(cmd));
    }
}

void Numeral::_i2cWrite(int ICAddress, int *data, int num) {
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

void Numeral::_updateLED(int numeral) {
    /* Update the live LED on/off values
     */
    _i2cWrite(i2cAddresses[numeral], ledState[numeral], sizeof(ledState[numeral]));
}

void Numeral::_updatePWM(int numeral) {
    /* Write the pending LED changes to the numeral.
     */
    _i2cWrite(i2cAddresses[numeral], pwmState[numeral], sizeof(pwmState[numeral]));
    _writeRegister(numeral, pwmRegisterUpdate, 0x00);
}

void Numeral::power(int numeral, bool shutdownState) {
    /* Turns the IC on and off for power savings
     *  
     * `numeral`
     *     The numeral to set
     *     
     * `shutdownState`
     *     Whether the IC should be on (1) or off (0)
     */
    _writeRegister(numeral, shutdownRegister, shutdownState);
}

void Numeral::update(void) {
    /* Update all connected numerals from ledState and pwmState.
     */
    for (int numeral = 0; numeral < count; numeral++) {
        _updateLED(numeral);
        _updatePWM(numeral);
    }
}

void Numeral::segment(int numeral, int segment, int r, int g, int b) {
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

void Numeral::writeDigit(int numeral, int digit, int r, int g, int b) {
    /* Display a digit (0-9) on a single numeral.
     *  
     * `numeral`
     *    The numeral to display this digit on
     *  
     * `digit`
     *    The digit to display
     *  
     * `r`
     *    How bright the red channel should be (0-255)
     *  
     * `g`
     *    How bright the green channel should be (0-255)
     *  
     * `b`
     *    How bright the blue channel should be (0-255)
     */
    for (int seg = segmentA; seg <= segmentG; seg++) {
        if (pgm_read_byte(&digitSegments[digit][seg])) {
            segment(numeral, seg, r, g, b);
        } else {
            segment(numeral, seg, 0, 0, 0);
        }
    }
  
    _updatePWM(numeral);
}

void Numeral::writeNumber(int number, int r, int g, int b) {
    /* Display a 1-4 digit number across all numerals.
     *
     * `number`
     *   The number to display. Numbers larger than 4 digits will be truncated
     *   to the last 4 digits.
     *
     * `r`
     *    How bright the red channel should be (0-255)
     *  
     * `g`
     *    How bright the green channel should be (0-255)
     *  
     * `b`
     *    How bright the blue channel should be (0-255)
     */
    int ones = 10;
    int tens = 10;
    int hundreds = 10;
    int thousands = 10;

    /* Split the int into up to 4 digits.
     */
    if (number < 10) {
        ones = number;
    } else if (number < 100) {
          tens = number / 10;
          ones = number % 10;
    } else if (number < 1000) {
          hundreds = number / 100;
          tens = (number - (hundreds*100)) / 10;
          ones = (number - (hundreds*100)) % 10;
    } else if (number < 10000) {
          thousands = number / 1000;
          hundreds = (number - (thousands*1000)) / 100;
          tens = (number - (thousands*1000) - (hundreds*100)) / 10;
          ones = (number - (thousands*1000) - (hundreds*100)) % 10;
    } else {
          thousands = number % 10000;
          hundreds = (number - (thousands*1000)) / 100;
          tens = (number - (thousands*1000) - (hundreds*100)) / 10;
          ones = (number - (thousands*1000) - (hundreds*100)) % 10;
    }

    writeDigit(i2cAddresses[count - 1], ones, r, g, b);
    if (count > 1) {
        writeDigit(i2cAddresses[count - 2], tens, r, g, b);
    }
    if (count > 2) {
        writeDigit(i2cAddresses[count - 3], hundreds, r, g, b);
    }
    if (count > 3) {
        writeDigit(i2cAddresses[count - 4], thousands, r, g, b);
    }
}
