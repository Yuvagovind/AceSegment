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

#ifndef ACE_SEGMENT_TM1637_DRIVER_FAST_H
#define ACE_SEGMENT_TM1637_DRIVER_FAST_H

#if defined(ARDUINO_ARCH_AVR)

#include <stdint.h>
#include <Arduino.h>
#include <digitalWriteFast.h>

namespace ace_segment {

/**
  * In theory, the chip should be able to handle fairly small delays, like
  * 250 kHz or 4 microseconds. But Many TM1637 LED Modules from eBay
  * apparently uses a low value pullup resistor, coupled with high valued
  * capacitor, so the rise time of the signal on these lines are slow. A
  * value of 50 microseconds does not work on my LED Modules, but 100 does
  * work.
  */
static const uint16_t kDefaultTm1637DelayMicros = 100;

/**
 * Exactly the same as Tm1637Driver except that this uses the `digitalWriteFast`
 * library on AVR processors. Normally, the digitalWriteFast library is used to
 * get faster speeds over `digitalWrite()` and `pinMode()` functions. However,
 * for the purposes of this library, speed is not the important factor. However,
 * the `digitalWriteFast` functions consume less flash memory, and that's the
 * main advantage.
 *
 * This class is stateless. It is thread-safe.
 */
template <
    uint8_t clockPin,
    uint8_t dioPin,
    uint16_t delayMicros = kDefaultTm1637DelayMicros
>
class Tm1637DriverFast {
  public:
    explicit Tm1637DriverFast() = default;

    void begin() const {
      // These are open-drain lines, with a pull-up resistor. We must not drive
      // them HIGH actively since that could damage the transitor at the other
      // end of the line pulling LOW. Instead, we go into INPUT mode to let the
      // line to HIGH through the pullup resistor, then go to OUTPUT mode only
      // to pull down.
      digitalWriteFast(clockPin, LOW);
      digitalWriteFast(dioPin, LOW);

      // Begin with both lines at HIGH.
      clockHigh();
      dataHigh();
    }

    /** Generate the I2C start condition. */
    void startCondition() const {
      clockHigh();
      dataHigh();

      dataLow();
      clockLow();
    }

    /** Generate the I2C stop condition. */
    void stopCondition() const {
      dataLow();
      clockHigh();
      dataHigh();
    }

    /**
     * Send the data byte on the data bus.
     * @return 0 means ACK, 1 means NACK.
     */
    uint8_t sendByte(uint8_t data) const {
      for (uint8_t i = 0;  i < 8; ++i) {
        if (data & 0x1) {
          dataHigh();
        } else {
          dataLow();
        }
        clockHigh();
        clockLow();
        data >>= 1;
      }

      // Device places the ACK/NACK bit upon the falling edge of the 8th CLK,
      // which happens in the loop above.
      pinModeFast(dioPin, INPUT);
      bitDelay();
      uint8_t ack = digitalReadFast(dioPin);

      // Device releases DIO upon falling edge of the 9th CLK.
      clockHigh();
      clockLow();
      return ack;
    }

  private:
    void bitDelay() const { delayMicroseconds(delayMicros); }

    void clockHigh() const { pinModeFast(clockPin, INPUT); bitDelay(); }

    void clockLow() const { pinModeFast(clockPin, OUTPUT); bitDelay(); }

    void dataHigh() const { pinModeFast(dioPin, INPUT); bitDelay(); }

    void dataLow() const { pinModeFast(dioPin, OUTPUT); bitDelay(); }
};

} // ace_segment

#endif // defined(ARDUINO_ARCH_AVR)

#endif
