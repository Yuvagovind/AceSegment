// This file was generated by the following script:
//   ../../tools/fast_driver.py --digit_pins 4 5 6 7 --segment_spi_pins 10 11 13 --class_name FastSpiDriver --output_files
//
// DO NOT EDIT

#ifdef __AVR__

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include "FastSpiDriver.h"

const uint8_t FastSpiDriver::kDigitPins[] = {
  4,
  5,
  6,
  7,
};

const FastSpiDriver::DigitalWriter FastSpiDriver::kDigitWriters[] = {
  digitalWriteFastDigit00Low,
  digitalWriteFastDigit00High,
  digitalWriteFastDigit01Low,
  digitalWriteFastDigit01High,
  digitalWriteFastDigit02Low,
  digitalWriteFastDigit02High,
  digitalWriteFastDigit03Low,
  digitalWriteFastDigit03High,
};

void FastSpiDriver::configure() {
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, OUTPUT);
    disableDigit(digit);
  }

  pinMode(kLatchPin, OUTPUT);
  pinMode(kDataPin, OUTPUT);
  pinMode(kClockPin, OUTPUT);

  SPI.begin();

  ace_segment::ModulatingDigitDriver::configure();
}

void FastSpiDriver::displayCurrentField() {
  if (mPreparedToSleep) return;

  bool isCurrentDigitOn;
  ace_segment::DimmablePattern& dimmablePattern =
      mDimmablePatterns[mCurrentDigit];
  uint8_t brightness = dimmablePattern.brightness;
  if (mCurrentDigit != mPrevDigit) {
    disableDigit(mPrevDigit);
    isCurrentDigitOn = false;
    mCurrentSubFieldMax = ((uint16_t) mNumSubFields * brightness) / 256;
  } else {
    isCurrentDigitOn = mIsPrevDigitOn;
  }

  if (brightness < 255 && mCurrentSubField >= mCurrentSubFieldMax) {
    if (isCurrentDigitOn) {
      disableDigit(mCurrentDigit);
      isCurrentDigitOn = false;
    }
  } else {
    if (!isCurrentDigitOn) {
      SegmentPatternType segmentPattern = dimmablePattern.pattern;
      if (segmentPattern != mSegmentPattern) {
        drawSegments(segmentPattern);
        mSegmentPattern = segmentPattern;
      }
      enableDigit(mCurrentDigit);
      isCurrentDigitOn = true;
    }
  }

  mCurrentSubField++;
  mPrevDigit = mCurrentDigit;
  mIsPrevDigitOn = isCurrentDigitOn;
  if (mCurrentSubField >= mNumSubFields) {
    ace_segment::Util::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }
}

void FastSpiDriver::drawSegments(uint8_t pattern) {
  digitalWriteFast(kLatchPin, LOW);
  uint8_t actualPattern = (kSegmentOn == HIGH) ? pattern : ~pattern;
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(actualPattern);
  SPI.endTransaction();
  digitalWriteFast(kLatchPin, HIGH);
}

void FastSpiDriver::prepareToSleep() {
  Driver::prepareToSleep();
  disableDigit(mPrevDigit);
}

#endif
