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

#ifndef ACE_SEGMENT_LED_MATRIX_SERIAL_H
#define ACE_SEGMENT_LED_MATRIX_SERIAL_H

#include "LedMatrixSplit.h"

namespace ace_segment {

class Hardware;

/**
 * An implementation of LedMatrixSplit with an 74HC595 Serial-To-Parallel
 * converter chip on the segment pins. The wiring is as follows:
 *
 *   latchPin/D10/SS -- ST_CP (Phillips) / RCK (TI) / Pin 12 (rising)
 *   dataPin/D11/MOSI -- DS (Phillips) / SER (TI) / Pin 14
 *   clockPin/D13/SCK -- SH_CP (Phillips) / SRCK (TI) / Pin 11 (rising)
 */
class LedMatrixSplitSerial: public LedMatrixSplit {
  public:
    LedMatrixSplitSerial(
        SpiAdapter* spiAdapter,
        bool cathodeOnGroup,
        bool transistorsOnGroups,
        bool transistorsOnElements,
        uint8_t numGroups,
        uint8_t numElements,
        const uint8_t* groupPins
    ) :
        mSpiAdapter(spiAdapter),
        LedMatrixSplit(
            cathodeOnGroup,
            transistorsOnGroups,
            transistorsOnElements,
            numGroups,
            numElements,
            groupPins)
    {}

    void configure() override {
      mSpiAdapter->spiBegin();
      LedMatrix::configure();
    }

    void finish() override {
      mSpiAdapter->spiEnd();
      LedMatrix::finish();
    }

    void drawElements(uint8_t pattern) override {
      uint8_t actualPattern = (mElementOn == HIGH) ? pattern : ~pattern;
      mSpiAdapter.transfer(actualPattern);
    }

  private:
    const SpiAdapter* const mSpiAdapter;
};

}
#endif
