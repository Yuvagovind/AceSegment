/*
 * A simple demo of a single HT16K33 LED module, with the digits [0,3] scrolling
 * to the left every second, and changing the brightness each iteration.
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // Ht16k33Module, LedDisplay

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_segment::Ht16k33Module;
using ace_segment::HardWireInterface;
using ace_segment::SimpleWireInterface;
using ace_segment::LedDisplay;

// Select the I2C implementation:
// Built-in Arduino <Wire.h>
#define WIRE_INTERFACE_TYPE_HARD 0
// https://github.com/Testato/SoftwareWire
#define WIRE_INTERFACE_TYPE_SOFTWARE_WIRE 1
// https://github.com/stevemarple/SoftWire
#define WIRE_INTERFACE_TYPE_SOFT_WIRE 2
// https://github.com/RaemondBW/SWire
#define WIRE_INTERFACE_TYPE_SWIRE 3
// AceSegment's own software Wire.
#define WIRE_INTERFACE_TYPE_SIMPLE_WIRE 4

const uint8_t HT16K33_I2C_ADDRESS = 0x70;

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_HT16K33
#endif

#if defined(EPOXY_DUINO)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_SIMPLE_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 5;

#elif defined(AUNITER_MICRO_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_SIMPLE_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#elif defined(AUNITER_STM32_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_HARD

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;

#elif defined(AUNITER_D1MINI_LARGE_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_HARD

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;

#elif defined(AUNITER_ESP32_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_HARD

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;

#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

#if WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_HARD
  #warning Using Wire.h
  #include <Wire.h>
  using WireInterface = HardWireInterface<TwoWire>;
  WireInterface wireInterface(Wire, HT16K33_I2C_ADDRESS);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SOFTWARE_WIRE
  #warning Using SoftwareWire.h
  #include <SoftwareWire.h>
  SoftwareWire softwareWire(SDA_PIN, SCL_PIN);
  using WireInterface = HardWireInterface<SoftwareWire>;
  WireInterface wireInterface(softwareWire, HT16K33_I2C_ADDRESS);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SOFT_WIRE
  #warning Using SoftWire.h
  #include <SoftWire.h>
  SoftWire softWire(SDA_PIN, SCL_PIN);
  using WireInterface = HardWireInterface<SoftWire>;
  WireInterface wireInterface(softWire, HT16K33_I2C_ADDRESS);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SWIRE
  #warning Using SWire.h
  #include <SWire.h>
  using WireInterface = HardWireInterface<SoftWire>;
  WireInterface wireInterface(SWire, HT16K33_I2C_ADDRESS);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SIMPLE_WIRE
  #warning Using SimpleWireInterface.h
  using WireInterface = SimpleWireInterface;
  WireInterface wireInterface(
      HT16K33_I2C_ADDRESS, SDA_PIN, SCL_PIN, DELAY_MICROS);
#else
  #error Unknown WIRE_INTERFACE_TYPE
#endif

Ht16k33Module<WireInterface, NUM_DIGITS> ht16k33Module(wireInterface);
LedDisplay display(ht16k33Module);

void setupAceSegment() {
#if WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_HARD
  Wire.begin();
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SOFTWARE_WIRE
  softwareWire.begin();
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SOFT_WIRE
  softWire.begin();
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SWIRE
  SWire.begin(SDA_PIN, SCL_PIN);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SIMPLE_WIRE
  // do nothing
#else
  #error Unknown WIRE_INTERFACE_TYPE
#endif

  wireInterface.begin();
  ht16k33Module.begin();
}

//----------------------------------------------------------------------------

// The HT16K33 controller supports up to 6 digits.
const uint8_t PATTERNS[6] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
};

TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightness = 0;

// Every second, scroll the display and change the brightness.
void updateDisplay() {
  static uint16_t prevChangeMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevChangeMillis) >= 1000) {
    prevChangeMillis = nowMillis;

    // Update the display
    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      display.writePatternAt(i, PATTERNS[j]);
      // Write a decimal point every other digit, for demo purposes.
      display.writeDecimalPointAt(i, j & 0x1);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness. The HT16K33 has 16 levels of brightness (0-15).
    display.setBrightness(brightness);
    incrementMod(brightness, (uint8_t) 16);
  }
}

// Every 100 ms, unconditionally flush() to the LED module which updates all
// digits.
void flushModule() {
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 100) {
    prevFlushMillis = nowMillis;

    // Flush incrementally, and measure the time.
    uint16_t startMicros = micros();
    ht16k33Module.flush();
    uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
    stats.update(elapsedMicros);
  }
}

// Every 5 seconds, print stats about how long flush() or flushIncremental()
// took.
void printStats() {
#if ENABLE_SERIAL_DEBUG >= 1
  static uint16_t prevStatsMillis;

  // Every 5 seconds, print out the statistics.
  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevStatsMillis) >= 5000) {
    prevStatsMillis = nowMillis;

    Serial.print("min/avg/max:");
    Serial.print(stats.getMin());
    Serial.print('/');
    Serial.print(stats.getAvg());
    Serial.print('/');
    Serial.println(stats.getMax());
    stats.reset();
  }
#endif
}

//-----------------------------------------------------------------------------

void setup() {
  delay(1000);

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200);
  while (!Serial);
#endif

  setupAceSegment();
}

void loop() {
  updateDisplay();

  flushModule();

  printStats();
}
