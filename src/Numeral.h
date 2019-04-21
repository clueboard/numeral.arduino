/*
 * Numeral.h - Arduino library for interfacing with the Clueboard Numeral devices.
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
#pragma once

#include "Arduino.h"

/* Possible i2c addresses for Numerals
 */
const int numeralAddressStart = 0x3b;
const int numeralAddressEnd = 0x3f;

/* Registers used by the is31fl3235a
 */
const int ledRegisterStart = 0x2A;       // on/off status for OUT1. The next 27 registers control OUT2-OUT28. (Values: 0-1)
const int ledRegisterEnd = 0x45;         // on/off status for OUT28. (Values: 0-1)
const int pwmRegisterStart = 0x05;       // Register for OUT1's PWM value. The next 27 are for OUT2-OUT28. (Values: 0-255)
const int pwmRegisterEnd = 0x20;         // Register for OUT28's PWM value. (Values: 0-255)
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
//   R   G   B
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

/* Our public API
 */
class Numeral {
    public:
        void i2cScan(void);
        void addNumeral(int i2cAddress);
        void setup(void);
        void writeDigit(int numeral, int digit, int r, int g, int b);
        void writeNumber(int number, int r, int g, int b);
        void segment(int numeral, int segment, int r, int g, int b);
        void power(int numeral, bool shutdownState);
        void update(void);
        int count;
        int i2cAddresses[4];
        int ledState[4][29] = {
            {ledRegisterStart,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
            {ledRegisterStart,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
            {ledRegisterStart,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
            {ledRegisterStart,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
        };
        int pwmState[4][29] = {
            {pwmRegisterStart,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {pwmRegisterStart,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {pwmRegisterStart,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {pwmRegisterStart,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
        };
    private:
        void _i2cWrite(int ICAddress, int *data, int num);
        void _updateLED(int numeral);
        void _updatePWM(int numeral);
        void _writeRegister(int numeral, int ICRegister, int data);
};


/* Automatically instaniate numeral
 */
#ifndef NUMERAL_CPP
Numeral numeral;
#endif
