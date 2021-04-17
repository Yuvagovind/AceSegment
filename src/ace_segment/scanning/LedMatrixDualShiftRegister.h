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

#ifndef ACE_SEGMENT_LED_MATRIX_DUAL_SHIFT_REGISTER_H
#define ACE_SEGMENT_LED_MATRIX_DUAL_SHIFT_REGISTER_H

#include "LedMatrixBase.h"

class LedMatrixDualShiftRegisterTest_draw;
class LedMatrixDualShiftRegisterTest_enableGroup;
class LedMatrixDualShiftRegisterTest_disableGroup;

namespace ace_segment {

/**
 * An LedMatrix that whose group pins are attached to one 74HC595 shift register
 * and the element pins are attached to another shift register. The 2 shift
 * registers are daisy chained so that they can be accessed in a serial transfer
 * of 16-bits using hardware or software SPI.
 *
 * The group pins are assumed to be connected to the most significant byte. The
 * element pins are connected to the least signficiant byte.
 *
 * @tparam SPII class providing SPI, either SwSpiInterface or HwSpiInterface
 */
template<typename SPII>
class LedMatrixDualShiftRegister: public LedMatrixBase {
  public:
    LedMatrixDualShiftRegister(
        const SPII& spiInterface,
        uint8_t groupOnPattern,
        uint8_t elementOnPattern
    ) :
        LedMatrixBase(groupOnPattern, elementOnPattern),
        mSpiInterface(spiInterface)
    {}

    void begin() const {}

    void end() const {}

    /**
     * Write out the group and element patterns in a single 16-bit stream
     * with the group bits in the MSB and the element bits in the LSB.
     */
    void draw(uint8_t group, uint8_t elementPattern) const {
      uint8_t groupPattern = 0x1 << group; // Would a lookup table be faster?

      uint8_t actualGroupPattern = (groupPattern ^ mGroupXorMask);
      uint8_t actualElementPattern = (elementPattern ^ mElementXorMask);

      mSpiInterface.transfer16(
          actualGroupPattern << 8 | actualElementPattern);
      mPrevElementPattern = elementPattern;
    }

    void enableGroup(uint8_t group) const {
      draw(group, mPrevElementPattern);
    }

    void disableGroup(uint8_t group) const {
      (void) group;
      clear();
    }

    void clear() const {
      uint8_t actualGroupPattern = 0x00 ^ mGroupXorMask;
      uint8_t actualElementPattern = 0x00 ^ mElementXorMask;
      mSpiInterface.transfer16(
          actualGroupPattern << 8 | actualElementPattern);
      mPrevElementPattern = 0x00;
    }

  private:
    friend class ::LedMatrixDualShiftRegisterTest_draw;
    friend class ::LedMatrixDualShiftRegisterTest_enableGroup;
    friend class ::LedMatrixDualShiftRegisterTest_disableGroup;

    const SPII& mSpiInterface;

    /**
     * Remember the previous element pattern to support disableGroup() and
     * enableGroup().
     */
    mutable uint8_t mPrevElementPattern;
};

}

#endif
