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

#ifndef ACE_SEGMENT_NUMBER_WRITER_H
#define ACE_SEGMENT_NUMBER_WRITER_H

#include <stdint.h>
#include "LedDisplay.h"

namespace ace_segment {

/**
 * The NumberWriter supports converting decimal and hexadecimal numbers to
 * segment patterns expected by LedDisplay. The character set includes 0 to F,
 * and a few other characters which should be self-explanatory: kSpace and
 * kMinus.
 */
class NumberWriter {
  public:
    /**
     * The type of the character set supported by many methods in this class,
     * usually containing the string "HexChar" in its name. This custom
     * character set is not ASCII to save flash memory. It is a restricted set
     * that starts with 0 and goes up to 0xF to support hexadecimal digits. In
     * addition, the character set adds few more characters for convenience:
     *
     *  * kSpace = 0x10 = space character
     *  * kMinus = 0x11 = minus character
     *
     * The `hexchar_t` typedef is an alias for `uint8_t` and unfortunately, C++
     * will not prevent mixing a normal `char` or `uint8_t` with a `hexchar_t`.
     * However, it does make the documentation of the various methods more
     * self-explanatory.
     */
    typedef uint8_t hexchar_t;

    /** Total number of characters in the HexCharacter set. */
    static const uint8_t kNumHexChars = 18;

    /** A space character. */
    static const hexchar_t kSpace = 0x10;

    /** A minus character. */
    static const hexchar_t kMinus = 0x11;

    /** Constructor. */
    explicit NumberWriter(LedDisplay& ledDisplay) :
        mLedDisplay(ledDisplay)
    {}

    /** Get the underlying LedDisplay. */
    LedDisplay& display() const {
      return mLedDisplay;
    }

    /**
     * Write the hex character `c` at position `pos`. If `c` falls outside the
     * valid range of the kHexCharPatterns set, a `kSpace` character is printed
     * instead.
     */
    void writeHexCharAt(uint8_t pos, hexchar_t c) {
      writeHexCharInternalAt(pos, ((uint8_t) c < kNumHexChars) ? c : kSpace);
    }

    /** Write the `len` hex characters given by `s` starting at `pos`. */
    void writeHexCharsAt(uint8_t pos, const hexchar_t s[], uint8_t len) {
      for (uint8_t i = 0; i < len; ++i) {
        writeHexCharAt(pos++, s[i]);
      }
    }

    /** Write the 2-digit (8-bit) hexadecimal byte 'b' at pos. */
    void writeHexByteAt(uint8_t pos, uint8_t b) {
      uint8_t low = (b & 0x0F);
      b >>= 4;
      uint8_t high = (b & 0x0F);

      writeHexCharInternalAt(pos++, high);
      writeHexCharInternalAt(pos++, low);
    }

    /** Write the 4 digit (16-bit) hexadecimal word at pos. */
    void writeHexWordAt(uint8_t pos, uint16_t w) {
      uint8_t low = (w & 0xFF);
      uint8_t high = (w >> 8) & 0xFF;
      writeHexByteAt(pos, high);
      writeHexByteAt(pos + 2, low);
    }

    /**
     * Write the 16-bit unsigned number `num` as a decimal number at pos.
     *
     * @param pos start position of the number
     * @param num unsigned decimal number, 0-65535
     * @param pad left padding character (default: kSpace)
     * @param boxSize size of box; 0 means no boxing; < 0 means left justified
     *    inside |boxSize|; > 0 means right justified inside |boxSize| (this is
     *    meant to be similar to the "%-5d" or "%5d" specifier to the printf()
     *    function)
     */
    void writeUnsignedDecimalAt(uint8_t pos, uint16_t num,
        hexchar_t pad = kSpace, int8_t boxSize = 0);

    /** Same as writeUnsignedDecimalAt() but prepends a '-' sign if negative. */
    void writeSignedDecimalAt(uint8_t pos, int16_t num,
        hexchar_t pad = kSpace, int8_t boxSize = 0);

  private:
    // disable copy-constructor and assignment operator
    NumberWriter(const NumberWriter&) = delete;
    NumberWriter& operator=(const NumberWriter&) = delete;

    /** Similar to writeHexCharAt() without performing bounds check. */
    void writeHexCharInternalAt(uint8_t pos, hexchar_t c);

    /** Similar to writeHexCharsAt() without performing bounds check. */
    void writeHexCharsInternalAt(uint8_t pos, const hexchar_t s[],
        uint8_t len) {
      for (uint8_t i = 0; i < len; ++i) {
        writeHexCharInternalAt(pos++, s[i]);
      }
    }

    /**
     * Convert the integer num to an array of HexChar in the provided buf, with
     * the least significant digit going to buf[bufSize-1], and then working
     * backwards to the most significant digit.
     *
     * @param num number to convert
     * @param buf buffer of hex characters
     * @param bufSize must be 5 or larger (largest uint16_t is 65535, plus
     *    an optional sign bit if called from a signed version)
     *
     * @return index into buf that points to the start of the converted number,
     * e.g. for a single digit number, the returned value will be `bufSize-1`.
     */
    uint8_t toDecimal(uint16_t num, hexchar_t buf[], uint8_t bufSize) {
      uint8_t pos = bufSize;
      while (true) {
        if (num < 10) {
          buf[--pos] = num;
          break;
        }
        uint16_t quot = num / 10;
        buf[--pos] = num - quot * 10;
        num = quot;
      }
      return pos;
    }

  private:
    /** Bit pattern map for hex characters. */
    static const uint8_t kHexCharPatterns[kNumHexChars];

    LedDisplay& mLedDisplay;
};

}

#endif
