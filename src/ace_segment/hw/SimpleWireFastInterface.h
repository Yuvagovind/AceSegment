/*
MIT License

Copyright (c) 2021 Brian T. Park

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

#ifndef ACE_SEGMENT_SIMPLE_WIRE_FAST_INTERFACE_H
#define ACE_SEGMENT_SIMPLE_WIRE_FAST_INTERFACE_H

// This header file requires the digitalWriteFast library on AVR, or the
// EpoxyMockDigitalWriteFast library on EpoxyDuino.
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)

#include <stdint.h>
#include <Arduino.h> // delayMicroseconds()

namespace ace_segment {

/**
 * A version of SimpleWriteInterface that uses one of the <digitalWriteFast.h>
 * libraries. The biggest benefit of using digitalWriteFast is the reduction of
 * flash memory size, 500-700 bytes on AVR.
 *
 * @tparam T_DATA_PIN
 * @tparam T_CLOCK_PIN
 * @tparam T_DELAY_MICROS delay after each bit transition (full cycle = 2 *
     delayMicros)
 */
template <
    uint8_t T_DATA_PIN,
    uint8_t T_CLOCK_PIN,
    uint16_t T_DELAY_MICROS
>
class SimpleWireFastInterface {
  public:
    /**
     * Constructor.
     * @param addr I2C address of slave device
     */
    SimpleWireFastInterface(uint8_t addr) : mAddr(addr) {}

    /** Initialize the clock and data pins. */
    void begin() {
      // These are open-drain lines, with a pull-up resistor. We must not drive
      // them HIGH actively since that could damage the transitor at the other
      // end of the line pulling LOW. Instead, we go into INPUT mode to let the
      // line to HIGH through the pullup resistor, then go to OUTPUT mode only
      // to pull down.
      digitalWriteFast(T_CLOCK_PIN, LOW);
      digitalWriteFast(T_DATA_PIN, LOW);

      // Begin with both lines at HIGH.
      clockHigh();
      dataHigh();
    }

    /** Set clock and data pins to INPUT mode. */
    void end() const {
      clockHigh();
      dataHigh();
    }

    /** Send start condition. */
    void beginTransmission() {
      clockHigh();
      dataHigh();

      dataLow();
      clockLow();

      // Send I2C addr (7 bits) and R/W bit set to "write" (0x00).
      uint8_t effectiveAddr = (mAddr << 1) | 0x00;
      write(effectiveAddr);
    }

    /**
     * Send the data byte on the data bus, with MSB first as specified by I2C.
     *
     * @return 0 means ACK, 1 means NACK.
     */
    uint8_t write(uint8_t data) {
      for (uint8_t i = 0;  i < 8; ++i) {
        if (data & 0x80) {
          dataHigh();
        } else {
          dataLow();
        }
        clockHigh();
        clockLow();
        data <<= 1;
      }

      // Device places the ACK/NACK bit upon the falling edge of the 8th CLK,
      // which happens in the loop above.
      pinModeFast(T_DATA_PIN, INPUT);
      bitDelay();
      uint8_t ack = digitalReadFast(T_DATA_PIN);

      // Device releases SDA upon falling edge of the 9th CLK.
      clockHigh();
      clockLow();
      return ack;
    }

    /** Send stop condition. */
    void endTransmission() {
      dataLow();
      clockHigh();
      dataHigh();
    }

  private:
    void bitDelay() const { delayMicroseconds(T_DELAY_MICROS); }

    void clockHigh() const { pinModeFast(T_CLOCK_PIN, INPUT); bitDelay(); }

    void clockLow() const { pinModeFast(T_CLOCK_PIN, OUTPUT); bitDelay(); }

    void dataHigh() const { pinModeFast(T_DATA_PIN, INPUT); bitDelay(); }

    void dataLow() const { pinModeFast(T_DATA_PIN, OUTPUT); bitDelay(); }

  private:
    uint8_t const mAddr;
};

}

#endif // defined(ARDUINO_ARCH_AVR)

#endif
