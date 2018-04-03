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

#ifndef ACE_SEGMENT_CHAR_WRITER_H
#define ACE_SEGMENT_CHAR_WRITER_H

#include <stdint.h>
#include "StyledDigit.h"
#include "Renderer.h"

namespace ace_segment {

/**
 * The CharWriter supports mapping of ASCII (0 - 127) characters to segment
 * patterns supported by Renderer.
 */
class CharWriter {
  public:
    /** Constructor. */
    explicit CharWriter(Renderer* renderer):
        mRenderer(renderer)
    {}

    /** Get the number of digits. */
    uint8_t getNumDigits() { return mRenderer->getNumDigits(); }

    /** Write the character at the specified position. */
    void writeCharAt(uint8_t digit, char c);

    /** Write the character at the specified position. */
    void writeCharAt(uint8_t digit, char c, StyledDigit::StyleType style);

    /** Write the style for a given digit, leaving character unchanged. */
    void writeStyleAt(uint8_t digit, StyledDigit::StyleType style) {
      if (digit >= getNumDigits()) return;
      mRenderer->writeStyleAt(digit, style);
    }

    /** Write the decimal point at digit. */
    void writeDecimalPointAt(uint8_t digit, bool state = true) {
      if (digit >= getNumDigits()) return;
      mRenderer->writeDecimalPointAt(digit, state);
    }

  private:
    static const uint8_t kNumCharacters = 128;
    // Bit pattern map for ASCII characters.
    static const uint8_t kCharacterArray[];

    // disable copy-constructor and assignment operator
    CharWriter(const CharWriter&) = delete;
    CharWriter& operator=(const CharWriter&) = delete;

    Renderer* const mRenderer;
};

}

#endif
