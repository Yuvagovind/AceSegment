# Copyright 2018 Brian T. Park
#
# MIT License
"""
Generate a version of ModulatingDigitDriver using digitalWriteFast()
with the segment pins connected through a 74HC595 serial-to-parallel chip.
Similar to LedMatrixSerial class.
"""

import logging


class DriverGenerator:
    HEADER_FILE = """\
// This file was generated by the following script:
//   {invocation}
//
// DO NOT EDIT

#include <stdint.h>
#include <digitalWriteFast.h>
#include <ace_segment/ModulatingDigitDriver.h>
#include <ace_segment/Util.h>

#ifndef ACE_SEGMENT_{class_name}_H
#define ACE_SEGMENT_{class_name}_H

class {class_name}: public ace_segment::ModulatingDigitDriver {{
  public:
    // Constructor
    {class_name}(ace_segment::DimmablePattern* dimmablePatterns,
            uint8_t numDigits, uint8_t numSubFields):
        ace_segment::ModulatingDigitDriver(
            nullptr /* ledMatrix */, dimmablePatterns, numDigits, numSubFields)
    {{}}

    // Destructor
    virtual ~{class_name}() override {{}}

    virtual void configure() override;
    virtual void displayCurrentField() override;
    virtual void prepareToSleep() override;

  private:
    typedef void (*DigitalWriter)(void);

    static const uint8_t kLatchPin = {latch_pin};
    static const uint8_t kDataPin = {data_pin};
    static const uint8_t kClockPin = {clock_pin};

    // define pin values depending on common cathode or anode wiring
    {on_off_constants}

    static const uint8_t kDigitPins[];
    static const DigitalWriter kDigitWriters[];

    static void disableDigit(uint8_t digit) {{
      uint8_t index = digit * 2 + kDigitOff;
      DigitalWriter writer = kDigitWriters[index];
      writer();
    }}

    static void enableDigit(uint8_t digit) {{
      uint8_t index = digit * 2 + kDigitOn;
      DigitalWriter writer = kDigitWriters[index];
      writer();
    }}

    static void drawSegments(uint8_t pattern) {{
      digitalWriteFast(kLatchPin, LOW);
      uint8_t actualPattern = (kSegmentOn == HIGH) ? pattern : ~pattern;
      shiftOutFast(actualPattern);
      digitalWriteFast(kLatchPin, HIGH);
    }}

    static void shiftOutFast(uint8_t pattern);

    // DigitalWriter functions for writing digit pins.
    {digit_writers}
}};

#endif
"""

    SOURCE_FILE = """\
// This file was generated by the following script:
//   {invocation}
//
// DO NOT EDIT

#include <stdint.h>
#include <Arduino.h>
#include <digitalWriteFast.h>
#include "{class_name}.h"

const uint8_t {class_name}::kDigitPins[] = {{
  {digit_pins}
}};

const {class_name}::DigitalWriter {class_name}::kDigitWriters[] = {{
  {digit_writers_array}
}};

void {class_name}::configure() {{
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {{
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, OUTPUT);
    disableDigit(digit);
  }}

  pinMode(kLatchPin, OUTPUT);
  pinMode(kDataPin, OUTPUT);
  pinMode(kClockPin, OUTPUT);

  ace_segment::ModulatingDigitDriver::configure();
}}

void {class_name}::displayCurrentField() {{
  if (mPreparedToSleep) return;

  bool isCurrentDigitOn;
  ace_segment::DimmablePattern& dimmablePattern =
      mDimmablePatterns[mCurrentDigit];
  uint8_t brightness = dimmablePattern.brightness;
  if (mCurrentDigit != mPrevDigit) {{
    disableDigit(mPrevDigit);
    isCurrentDigitOn = false;
    mCurrentSubFieldMax = ((uint16_t) mNumSubFields * brightness) / 256;
  }} else {{
    isCurrentDigitOn = mIsPrevDigitOn;
  }}

  if (brightness < 255 && mCurrentSubField >= mCurrentSubFieldMax) {{
    if (isCurrentDigitOn) {{
      disableDigit(mCurrentDigit);
      isCurrentDigitOn = false;
    }}
  }} else {{
    if (!isCurrentDigitOn) {{
      SegmentPatternType segmentPattern = dimmablePattern.pattern;
      if (segmentPattern != mSegmentPattern) {{
        drawSegments(segmentPattern);
        mSegmentPattern = segmentPattern;
      }}
      enableDigit(mCurrentDigit);
      isCurrentDigitOn = true;
    }}
  }}

  mCurrentSubField++;
  mPrevDigit = mCurrentDigit;
  mIsPrevDigitOn = isCurrentDigitOn;
  if (mCurrentSubField >= mNumSubFields) {{
    ace_segment::Util::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }}
}}

void {class_name}::shiftOutFast(uint8_t pattern) {{
  uint8_t mask = 0x80;
  for (uint8_t i = 0; i < 8; i++)  {{
    digitalWriteFast(kClockPin, LOW);
    if (pattern & mask) {{
      digitalWriteFast(kDataPin, HIGH);
    }} else {{
      digitalWriteFast(kDataPin, LOW);
    }}
    digitalWriteFast(kClockPin, HIGH);
    mask >>= 1;
  }}
}}

void {class_name}::prepareToSleep() {{
  Driver::prepareToSleep();
  disableDigit(mPrevDigit);
}}
"""

    def __init__(self, invocation, class_name, segment_serial_pins, digit_pins,
                 common_cathode, output_header, output_source, output_files,
                 digital_write_fast):
        if len(segment_serial_pins) != 3:
            logging.error("Must provide (latch, data, clock) pins")
            sys.exit(1)
        self.invocation = invocation
        self.class_name = class_name
        self.latch = segment_serial_pins[0]
        self.data = segment_serial_pins[1]
        self.clock = segment_serial_pins[2]
        self.digit_pins = digit_pins
        self.common_cathode = common_cathode
        self.output_header = output_header
        self.output_source = output_source
        self.output_files = output_files
        self.digital_write_fast = digital_write_fast
        logging.info("invocation: %s", self.invocation)
        logging.info("class_name: %s", self.class_name)
        logging.info("segment_serial_pins: %s", segment_serial_pins)
        logging.info("digit_pins: %s", self.digit_pins)
        logging.info("common_cathode: %s", self.common_cathode)
        logging.info("digital_write_fast: %s", self.digital_write_fast)

    def run(self):
        header = self.HEADER_FILE.format(
            invocation=self.invocation,
            class_name=self.class_name,
            latch_pin=self.latch,
            data_pin=self.data,
            clock_pin=self.clock,
            on_off_constants=self.get_on_off_constants(),
            digit_writers=self.get_digit_writers())
        if self.output_header:
            print(header)
        if self.output_files:
            header_filename = self.class_name + ".h"
            with open(header_filename, 'w', encoding='utf-8') as header_file:
                print(header, end='', file=header_file)
            logging.info("Created %s", header_filename)

        source = self.SOURCE_FILE.format(
            invocation=self.invocation,
            class_name=self.class_name,
            digit_pins=self.get_digit_pins_array(),
            digit_writers_array=self.get_digit_writers_array())
        if self.output_source:
            print(source)
        if self.output_files:
            source_filename = self.class_name + ".cpp"
            with open(source_filename, 'w', encoding='utf-8') as source_file:
                print(source, end='', file=source_file)
            logging.info("Created %s", source_filename)

    def get_digit_pins_array(self):
        entries = []
        for pin in self.digit_pins:
            entry = ('%s,' % (pin))
            entries.append(entry)
        return '\n  '.join(entries)

    def get_digit_writers(self):
        writers = []
        if self.digital_write_fast:
            method = 'digitalWriteFast'
        else:
            method = 'digitalWrite'
        for i in range(len(self.digit_pins)):
            pin = self.digit_pins[i]
            low = ('static void digitalWriteFastDigit%02dLow() ' +
                   '{ %s(%s, LOW); }') % (i, method, pin)
            high = ('static void digitalWriteFastDigit%02dHigh() ' +
                    '{ %s(%s, HIGH); }') % (i, method, pin)
            writers.append(low)
            writers.append(high)
        return '\n    '.join(writers)

    def get_digit_writers_array(self):
        entries = []
        for i in range(len(self.digit_pins)):
            low = ('digitalWriteFastDigit%02dLow,' % (i))
            high = ('digitalWriteFastDigit%02dHigh,' % (i))
            entries.append(low)
            entries.append(high)
        return '\n  '.join(entries)

    def get_on_off_constants(self):
        constants = []
        if self.common_cathode:
            constants.append('static const uint8_t kDigitOn = LOW;')
            constants.append('static const uint8_t kDigitOff = HIGH;')
            constants.append('static const uint8_t kSegmentOn = HIGH;')
            constants.append('static const uint8_t kSegmentOff = LOW;')
        else:
            constants.append('static const uint8_t kDigitOn = HIGH;')
            constants.append('static const uint8_t kDigitOff = LOW;')
            constants.append('static const uint8_t kSegmentOn = LOW;')
            constants.append('static const uint8_t kSegmentOff = HIGH;')
        return '\n    '.join(constants)
