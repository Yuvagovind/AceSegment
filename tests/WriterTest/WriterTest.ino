#line 2 "WriterTest.ino"

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

#include <stdarg.h>
#include <AUnit.h>
#include <AceSegment.h>
#include <ace_segment/testing/FakeRenderer.h>

using namespace aunit;
using namespace ace_segment;
using namespace ace_segment::testing;

const int8_t NUM_DIGITS = 4;

// create NUM_DIGITS+1 elements for doing array bound checking
DimmingDigit dimmingDigits[NUM_DIGITS + 1];
StyledDigit styledDigits[NUM_DIGITS + 1];

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): start"));

  Serial.println(F("setup(): end"));
}

void loop() {
  TestRunner::run();
}

void clearStyledDigits() {
  for (int i = 0; i < NUM_DIGITS; i++) {
    styledDigits[i].pattern = 0;
    styledDigits[i].style = 0;
  }
}

// ----------------------------------------------------------------------
// Tests for CharWriter.
// ----------------------------------------------------------------------

class CharWriterTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();

      mRenderer = new FakeRenderer(styledDigits, NUM_DIGITS);
      mRenderer->configure();
      mCharWriter = new CharWriter(mRenderer);
      clearStyledDigits();
    }

    virtual void teardown() override {
      delete mCharWriter;
      delete mRenderer;

      TestOnce::teardown();
    }

    FakeRenderer *mRenderer;
    CharWriter *mCharWriter;
};

testF(CharWriterTest, getNumDigits) {
  assertEqual(NUM_DIGITS, mCharWriter->getNumDigits());
}

testF(CharWriterTest, writeAt) {
  mCharWriter->writeCharAt(0, '0');
  assertEqual(0b00111111, styledDigits[0].pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigits[0].style);

  mCharWriter->writeCharAt(1, '0', StyledDigit::kStyleBlinkSlow);
  assertEqual(0b00111111, styledDigits[1].pattern);
  assertEqual(StyledDigit::kStyleBlinkSlow, styledDigits[1].style);

  mCharWriter->writeStyleAt(1, StyledDigit::kStyleBlinkFast);
  assertEqual(StyledDigit::kStyleBlinkFast, styledDigits[1].style);

  mCharWriter->writeDecimalPointAt(1);
  assertEqual(0b00111111 | 0x80, styledDigits[1].pattern);

  mCharWriter->writeDecimalPointAt(1, false);
  assertEqual(0b00111111, styledDigits[1].pattern);
}

testF(CharWriterTest, writeAt_outOfBounds) {
  styledDigits[4].pattern = 0;
  styledDigits[4].style = StyledDigit::kStyleNormal;
  mCharWriter->writeCharAt(4, 'a', StyledDigit::kStyleBlinkSlow);
  mCharWriter->writeDecimalPointAt(4);
  assertEqual(0, styledDigits[4].pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigits[4].style);
}

// ----------------------------------------------------------------------
// Tests for StringWriter.
// ----------------------------------------------------------------------

class StringWriterTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();

      mRenderer = new FakeRenderer(styledDigits, NUM_DIGITS);
      mRenderer->configure();
      mCharWriter = new CharWriter(mRenderer);
      mStringWriter = new StringWriter(mCharWriter);
      clearStyledDigits();
    }

    virtual void teardown() override {
      delete mStringWriter;
      delete mCharWriter;
      delete mRenderer;

      TestOnce::teardown();
    }

    void assertStyledDigitsEqual(int n, ...) {
      va_list args;
      va_start(args, n);
      for (int i = 0; i < n; i++) {
        uint8_t pattern = va_arg(args, int);
        uint8_t style = va_arg(args, int);
        const StyledDigit& styledDigit = styledDigits[i];
        assertEqual(pattern, styledDigit.pattern);
        assertEqual(style, styledDigit.style);
      }
      va_end(args);
    }

    FakeRenderer *mRenderer;
    CharWriter *mCharWriter;
    StringWriter *mStringWriter;
};

testF(StringWriterTest, writeStringAt) {
  mStringWriter->writeStringAt(0, ".1.2.3");

  // Should ge (".", "1.", "2.", "3") as the 4 digits
  assertStyledDigitsEqual(4,
    0b10000000, StyledDigit::kStyleNormal,
    0b00000110 | 0x80, StyledDigit::kStyleNormal,
    0b01011011 | 0x80, StyledDigit::kStyleNormal,
    0b01001111, StyledDigit::kStyleNormal);
}

