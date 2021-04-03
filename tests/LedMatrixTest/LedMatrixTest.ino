#line 2 "LedMatrixTest.ino"

/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>
#include <ace_segment/testing/TestableHardware.h>
#include <ace_segment/testing/TestableSpiAdapter.h>

using aunit::TestRunner;
using aunit::TestOnce;
using namespace ace_segment;
using namespace ace_segment::testing;

const int8_t NUM_DIGITS = 4;
const int8_t NUM_SEGMENTS = 8;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {0, 1, 2, 3};
const uint8_t SEGMENT_PINS[8] = {4, 5, 6, 7, 8, 9, 10, 11};

// Common Cathode, with transistors on Group pins
TestableHardware hardware;
LedMatrixDirect<TestableHardware> ledMatrixDirect(
    hardware,
    LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
    LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS,
    NUM_SEGMENTS,
    SEGMENT_PINS);

// Common Cathode, with transistors on Group pins
TestableSpiAdapter spiAdapter;
LedMatrixSingleShiftRegister<TestableHardware, TestableSpiAdapter>
  ledMatrixSingleShiftRegister(
    hardware,
    spiAdapter,
    LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
    LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS);

// Common Cathode, with transistors on Group pins
LedMatrixDualShiftRegister<TestableSpiAdapter> ledMatrixDualShiftRegister(
    spiAdapter,
    LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
    LedMatrixBase::kActiveHighPattern /*elementOnPattern*/);

// ----------------------------------------------------------------------
// Tests for LedMatrixSplitDirect.
// ----------------------------------------------------------------------

class LedMatrixDirectTest : public TestOnce {
  protected:
    void setup() override {
      ledMatrixDirect.begin();
      hardware.mEventLog.clear();
    }
};

testF(LedMatrixDirectTest, begin) {
  ledMatrixDirect.begin();
  assertEqual(24, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(24,
      EventType::kPinMode, 0, OUTPUT,
      EventType::kDigitalWrite, 0, LOW,
      EventType::kPinMode, 1, OUTPUT,
      EventType::kDigitalWrite, 1, LOW,
      EventType::kPinMode, 2, OUTPUT,
      EventType::kDigitalWrite, 2, LOW,
      EventType::kPinMode, 3, OUTPUT,
      EventType::kDigitalWrite, 3, LOW,

      EventType::kPinMode, 4, OUTPUT,
      EventType::kDigitalWrite, 4, LOW,
      EventType::kPinMode, 5, OUTPUT,
      EventType::kDigitalWrite, 5, LOW,
      EventType::kPinMode, 6, OUTPUT,
      EventType::kDigitalWrite, 6, LOW,
      EventType::kPinMode, 7, OUTPUT,
      EventType::kDigitalWrite, 7, LOW,
      EventType::kPinMode, 8, OUTPUT,
      EventType::kDigitalWrite, 8, LOW,
      EventType::kPinMode, 9, OUTPUT,
      EventType::kDigitalWrite, 9, LOW,
      EventType::kPinMode, 10, OUTPUT,
      EventType::kDigitalWrite, 10, LOW,
      EventType::kPinMode, 11, OUTPUT,
      EventType::kDigitalWrite, 11, LOW
  ));
}

testF(LedMatrixDirectTest, end) {
  ledMatrixDirect.end();
  assertEqual(12, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(12,
      EventType::kPinMode, 0, INPUT,
      EventType::kPinMode, 1, INPUT,
      EventType::kPinMode, 2, INPUT,
      EventType::kPinMode, 3, INPUT,
      EventType::kPinMode, 4, INPUT,
      EventType::kPinMode, 5, INPUT,
      EventType::kPinMode, 6, INPUT,
      EventType::kPinMode, 7, INPUT,
      EventType::kPinMode, 8, INPUT,
      EventType::kPinMode, 9, INPUT,
      EventType::kPinMode, 10, INPUT,
      EventType::kPinMode, 11, INPUT
  ));
}

testF(LedMatrixDirectTest, enableGroup) {
  ledMatrixDirect.enableGroup(1);
  assertEqual(1, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(1,
      EventType::kDigitalWrite, 1, HIGH));
}

testF(LedMatrixDirectTest, disableGroup) {
  ledMatrixDirect.disableGroup(1);
  assertEqual(1, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(1,
      EventType::kDigitalWrite, 1, LOW));
}

testF(LedMatrixDirectTest, drawElements) {
  ledMatrixDirect.drawElements(0x55);
  assertEqual(8, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(8,
      EventType::kDigitalWrite, 4, HIGH,
      EventType::kDigitalWrite, 5, LOW,
      EventType::kDigitalWrite, 6, HIGH,
      EventType::kDigitalWrite, 7, LOW,
      EventType::kDigitalWrite, 8, HIGH,
      EventType::kDigitalWrite, 9, LOW,
      EventType::kDigitalWrite, 10, HIGH,
      EventType::kDigitalWrite, 11, LOW
  ));
}

// ----------------------------------------------------------------------
// Tests for LedMatrixSingleShiftRegister.
// ----------------------------------------------------------------------

class LedMatrixSingleShiftRegisterTest : public TestOnce {
  protected:
    void setup() override {
      ledMatrixSingleShiftRegister.begin();
      hardware.mEventLog.clear();
    }
};

testF(LedMatrixSingleShiftRegisterTest, begin) {
  ledMatrixSingleShiftRegister.begin();
  assertEqual(8, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(8,
      EventType::kPinMode, 0, OUTPUT,
      EventType::kDigitalWrite, 0, LOW,
      EventType::kPinMode, 1, OUTPUT,
      EventType::kDigitalWrite, 1, LOW,
      EventType::kPinMode, 2, OUTPUT,
      EventType::kDigitalWrite, 2, LOW,
      EventType::kPinMode, 3, OUTPUT,
      EventType::kDigitalWrite, 3, LOW));
}

testF(LedMatrixSingleShiftRegisterTest, end) {
  ledMatrixSingleShiftRegister.end();
  assertEqual(4, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(4,
      EventType::kPinMode, 0, INPUT,
      EventType::kPinMode, 1, INPUT,
      EventType::kPinMode, 2, INPUT,
      EventType::kPinMode, 3, INPUT));

}

testF(LedMatrixSingleShiftRegisterTest, enableGroup) {
  ledMatrixSingleShiftRegister.enableGroup(1);
  assertEqual(1, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(1,
      EventType::kDigitalWrite, 1, HIGH));
}

testF(LedMatrixSingleShiftRegisterTest, disableGroup) {
  ledMatrixSingleShiftRegister.disableGroup(1);
  assertEqual(1, hardware.mEventLog.getNumRecords());
  assertTrue(hardware.mEventLog.assertEvents(1,
      EventType::kDigitalWrite, 1, LOW));
}

testF(LedMatrixSingleShiftRegisterTest, drawElements) {
  ledMatrixSingleShiftRegister.drawElements(0x55);
  assertEqual(1, spiAdapter.mEventLog.getNumRecords());
  assertTrue(spiAdapter.mEventLog.assertEvents(1,
      EventType::kSpiTransfer, 0x55
  ));
}

// ----------------------------------------------------------------------
// Tests for LedMatrixSplitSpi.
// ----------------------------------------------------------------------

class LedMatrixDualShiftRegisterTest : public TestOnce {
  protected:
    void setup() override {
      ledMatrixDualShiftRegister.begin();
      spiAdapter.mEventLog.clear();
    }
};

testF(LedMatrixDualShiftRegisterTest, begin) {
  ledMatrixDualShiftRegister.begin();
  assertEqual(0, spiAdapter.mEventLog.getNumRecords());
}

testF(LedMatrixDualShiftRegisterTest, end) {
  ledMatrixDualShiftRegister.end();
  assertEqual(0, spiAdapter.mEventLog.getNumRecords());
}

testF(LedMatrixDualShiftRegisterTest, enableGroup) {
  ledMatrixDualShiftRegister.mPrevElementPattern = 0x42;
  ledMatrixDualShiftRegister.enableGroup(1);

  assertEqual(1, spiAdapter.mEventLog.getNumRecords());
  uint16_t expectedOutput = ((0x1 << 1) << 8) | 0x42;
  assertTrue(spiAdapter.mEventLog.assertEvents(1,
      EventType::kSpiTransfer16, expectedOutput));
}

testF(LedMatrixDualShiftRegisterTest, disableGroup) {
  ledMatrixDualShiftRegister.disableGroup(2);

  assertEqual(1, spiAdapter.mEventLog.getNumRecords());
  uint16_t expectedOutput = 0x0000;
  assertTrue(spiAdapter.mEventLog.assertEvents(1,
      EventType::kSpiTransfer16, expectedOutput));
}

testF(LedMatrixDualShiftRegisterTest, draw) {
  ledMatrixDualShiftRegister.draw(3, 0x55);

  uint16_t expectedOutput = ((0x1 << 3) << 8) | 0x55;
  assertEqual(1, spiAdapter.mEventLog.getNumRecords());
  assertTrue(spiAdapter.mEventLog.assertEvents(1,
    EventType::kSpiTransfer16, expectedOutput
  ));
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
