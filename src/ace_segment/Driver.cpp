/*
MIT License

Copyright (c) 2018 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Arduino.h> // LOW and HIGH
#include "LedMatrix.h"
#include "Driver.h"

namespace ace_segment {

Driver::~Driver() {
  if (mLedMatrix) {
    delete mLedMatrix;
  }
}

void Driver::configure() {
  if (mLedMatrix) {
    mLedMatrix->configure();
  }
}

void Driver::setPattern(uint8_t digit, SegmentPatternType pattern,
    uint8_t brightness) {
  if (digit >= mNumDigits) return;
  DimmingDigit& dimmingDigit = mDimmingDigits[digit];
  dimmingDigit.pattern = pattern;
  dimmingDigit.brightness = brightness;
}

void Driver::setBrightness(uint8_t digit, uint8_t brightness) {
  if (digit >= mNumDigits) return;
  mDimmingDigits[digit].brightness = brightness;
}

}
