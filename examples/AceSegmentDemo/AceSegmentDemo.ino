#include <Arduino.h>
#include <AceButton.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_segment/hw/SoftSpiFastInterface.h>
#include <ace_segment/hw/HardSpiFastInterface.h>
#include <ace_segment/hw/SoftTmiFastInterface.h>
#include <ace_segment/scanning/LedMatrixDirectFast4.h>
#include <ace_segment/direct/DirectFast4Module.h>
#endif

using ace_common::incrementMod;
using namespace ace_segment;
using namespace ace_button;

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

// Use polling or interrupt.
#define USE_INTERRUPT 0

#if USE_INTERRUPT
  #include <TimerOne.h>
#endif

// Type of LED Module
#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_TM1637 1
#define LED_DISPLAY_TYPE_MAX7219 2
#define LED_DISPLAY_TYPE_HC595 3
#define LED_DISPLAY_TYPE_DIRECT 4
#define LED_DISPLAY_TYPE_HYBRID 5
#define LED_DISPLAY_TYPE_FULL 6

// Used by LED_DISPLAY_TYPE_SCANNING
#define LED_MATRIX_MODE_NONE 0
#define LED_MATRIX_MODE_DIRECT 1
#define LED_MATRIX_MODE_DIRECT_FAST 2
#define LED_MATRIX_MODE_SINGLE_SOFT_SPI 3
#define LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST 4
#define LED_MATRIX_MODE_SINGLE_HARD_SPI 5
#define LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST 6
#define LED_MATRIX_MODE_DUAL_SOFT_SPI 7
#define LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST 8
#define LED_MATRIX_MODE_DUAL_HARD_SPI 9
#define LED_MATRIX_MODE_DUAL_HARD_SPI_FAST 10

// Used by LED_DISPLAY_TYPE_DIRECT
#define DIRECT_INTERFACE_TYPE_NORMAL 0
#define DIRECT_INTERFACE_TYPE_FAST_4 1

// Used by LED_DISPLAY_TYPE_HC595, LED_DISPLAY_TYPE_HYBRID,
// and LED_DISPLAY_TYPE_FULL
#define INTERFACE_TYPE_SOFT_SPI 0
#define INTERFACE_TYPE_SOFT_SPI_FAST 1
#define INTERFACE_TYPE_HARD_SPI 2
#define INTERFACE_TYPE_HARD_SPI_FAST 3
#define INTERFACE_TYPE_SOFT_TMI 4
#define INTERFACE_TYPE_SOFT_TMI_FAST 5

// Some microcontrollers have 2 or more SPI buses. PRIMARY selects the default.
// SECONDARY selects the alternate. I don't have a board with more than 2, but
// we could add additional options here if needed.
#define SPI_INSTANCE_TYPE_PRIMARY 0
#define SPI_INSTANCE_TYPE_SECONDARY 1

// Button options: either digital buttons using ButtonConfig, 2 analog buttons
// using LadderButtonConfig, or 4 analog buttons using LadderButtonConfig:
//  * AVR: 10-bit analog pin
//  * ESP8266: 10-bit analog pin
//  * ESP32: 12-bit analog pin
#define BUTTON_TYPE_DIGITAL 0
#define BUTTON_TYPE_ANALOG 1

// Select the TM1637Module flush() method
#define TM_FLUSH_METHOD_NORMAL 0
#define TM_FLUSH_METHOD_INCREMENTAL 1

//------------------------------------------------------------------
// Hardware configuration.
//------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_TM1637
#endif

// Pro Micro dev board buttons are now hardwared to A2 and A3, instead of being
// configured with dip switches to either (2,3) or (8,9). Since (2,3) are used
// by I2C, and LED_MATRIX_MODE_DIRECT uses (8,9) pins for two of the LED
// segments/digits, the only spare pins are A2 and A3. All other digital pins
// are taken. Fortunately, the ATmega32U4 allows all analog pins to be used as
// digital pins.
#if defined(EPOXY_DUINO)
  // For EpoxyDuino, the actual numbers don't matter, so let's set them to (2,3)
  // since I'm not sure if A2 and A3 are defined.
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = 2;
  const uint8_t CHANGE_BUTTON_PIN = 3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  const uint8_t NUM_DIGITS = 4;
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI_FAST

#elif defined(AUNITER_MICRO_SCANNING_DIRECT)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_MICRO_SCANNING_SINGLE)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SOFT_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_SCANNING_DUAL)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_CUSTOM_DIRECT)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_DIRECT
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;

  // Choose one of the following variants:
  //#define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_NORMAL
  #define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_FAST
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_MICRO_CUSTOM_SINGLE)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HYBRID
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_CUSTOM_DUAL)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_FULL
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_TMI
  #define INTERFACE_TYPE INTERFACE_TYPE_SOFT_TMI_FAST
  const uint8_t CLK_PIN = A0;
  const uint8_t DIO_PIN = 9;
  const uint16_t BIT_DELAY = 100;

  // Select one of the flush methods.
  //#define TM_FLUSH_METHOD TM_FLUSH_METHOD_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_INCREMENTAL

#elif defined(AUNITER_MICRO_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_STM32_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = PA0;
  const uint8_t CHANGE_BUTTON_PIN = PA1;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SOFT_TMI
  const uint8_t CLK_PIN = PB3;
  const uint8_t DIO_PIN = PB4;
  const uint16_t BIT_DELAY = 100;

  // Select one of the flush methods.
  //#define TM_FLUSH_METHOD TM_FLUSH_METHOD_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_INCREMENTAL

#elif defined(AUNITER_STM32_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = PA0;
  const uint8_t CHANGE_BUTTON_PIN = PA1;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY

  #if SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_PRIMARY
    // SPI1 pins (default)
    const uint8_t LATCH_PIN = SS;
    const uint8_t DATA_PIN = MOSI;
    const uint8_t CLOCK_PIN = SCK;
  #elif SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_SECONDARY
    // SPI2 pins
    const uint8_t LATCH_PIN = PB12;
    const uint8_t DATA_PIN = PB15;
    const uint8_t CLOCK_PIN = PB13;
    SPIClass SPISecondary(DATA_PIN, PB14 /*miso*/, CLOCK_PIN);
  #else
    #error Unknown SPI_INSTANCE_TYPE
  #endif

#elif defined(AUNITER_STM32_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = PA0;
  const uint8_t CHANGE_BUTTON_PIN = PA1;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY

  #if SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_PRIMARY
    // SPI1 pins (default)
    const uint8_t LATCH_PIN = SS;
    const uint8_t DATA_PIN = MOSI;
    const uint8_t CLOCK_PIN = SCK;
  #elif SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_SECONDARY
    // SPI2 pins
    const uint8_t LATCH_PIN = PB12;
    const uint8_t DATA_PIN = PB15;
    const uint8_t CLOCK_PIN = PB13;
    SPIClass SPISecondary(DATA_PIN, PB14 /*miso*/, CLOCK_PIN);
  #else
    #error Unknown SPI_INSTANCE_TYPE
  #endif

#elif defined(AUNITER_D1MINI_LARGE_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 2;
  #define ANALOG_BUTTON_PIN A0
  #define ANALOG_BUTTON_LEVELS { \
      0 /*short to ground*/, \
      327 /*32%, 4.7k*/, \
      512 /*50%, 10k*/, \
      844 /*82%, 47k*/, \
      1023 /*100%, open*/ \
    }

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SOFT_TMI
  const uint8_t CLK_PIN = D5;
  const uint8_t DIO_PIN = D7;
  const uint16_t BIT_DELAY = 100;

#elif defined(AUNITER_D1MINI_LARGE_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 2;
  #define ANALOG_BUTTON_PIN A0
  #define ANALOG_BUTTON_LEVELS { \
      0 /*short to ground*/, \
      327 /*32%, 4.7k*/, \
      512 /*50%, 10k*/, \
      844 /*82%, 47k*/, \
      1023 /*100%, open*/ \
    }

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_D1MINI_LARGE_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 2;
  #define ANALOG_BUTTON_PIN A0
  #define ANALOG_BUTTON_LEVELS { \
      0 /*short to ground*/, \
      327 /*32%, 4.7k*/, \
      512 /*50%, 10k*/, \
      844 /*82%, 47k*/, \
      1023 /*100%, open*/ \
    }

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_ESP32_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 1;
  #define ANALOG_BUTTON_LEVELS {0, 2048, 4095}
  #define ANALOG_BUTTON_PIN A10

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SOFT_TMI
  const uint8_t CLK_PIN = 14;
  const uint8_t DIO_PIN = 13;
  const uint16_t BIT_DELAY = 100;

#elif defined(AUNITER_ESP32_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 1;
  #define ANALOG_BUTTON_LEVELS {0, 2048, 4095}
  #define ANALOG_BUTTON_PIN A10

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  // My dev board uses HSPI.
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_SECONDARY

  #if SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_PRIMARY
    // VSPI pins (default)
    const uint8_t LATCH_PIN = SS;
    const uint8_t DATA_PIN = MOSI;
    const uint8_t CLOCK_PIN = SCK;
  #elif SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_SECONDARY
    // HSPI pins
    const uint8_t LATCH_PIN = 15;
    const uint8_t DATA_PIN = 13;
    const uint8_t CLOCK_PIN = 14;
    SPIClass SPISecondary(HSPI);
  #else
    #error Unknown SPI_INSTANCE_TYPE
  #endif

#elif defined(AUNITER_ESP32_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 1;
  #define ANALOG_BUTTON_LEVELS {0, 2048, 4095}
  #define ANALOG_BUTTON_PIN A10

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  // My dev board uses HSPI.
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_SECONDARY

  #if SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_PRIMARY
    // VSPI pins (default)
    const uint8_t LATCH_PIN = SS;
    const uint8_t DATA_PIN = MOSI;
    const uint8_t CLOCK_PIN = SCK;
  #elif SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_SECONDARY
    // HSPI pins
    const uint8_t LATCH_PIN = 15;
    const uint8_t DATA_PIN = 13;
    const uint8_t CLOCK_PIN = 14;
    SPIClass SPISecondary(HSPI);
  #else
    #error Unknown SPI_INSTANCE_TYPE
  #endif

#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// Configurations for AceSegment
//------------------------------------------------------------------

// Total fields/second
//      = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//      = 60 * 1 * 4
//      = 240 fields/sec
//      => 4167 micros/field
//
// According to AutoBenchmark, *all* versions of ScanningModule with all
// configurations of LedMatrix can render a single field less than 304
// microseconds on a 16 MHz AVR processor.
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 1;

// The chain of resources.
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
    // Common Anode, with transitors on Group pins
    using LedMatrix = LedMatrixDirect<>;
    LedMatrix ledMatrix(
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        NUM_SEGMENTS,
        SEGMENT_PINS,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT_FAST
    // Common Anode, with transitors on Group pins
    using LedMatrix = LedMatrixDirectFast4<
      8, 9, 10, 16, 14, 18, 19, 15,
      4, 5, 6, 7
    >;
    LedMatrix ledMatrix(
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI
    // Common Cathode, with transistors on Group pins
    SoftSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixSingleHc595<SoftSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS):
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST
    // Common Cathode, with transistors on Group pins
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixSingleHc595<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI
    // Common Cathode, with transistors on Group pins
    HardSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixSingleHc595<HardSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST
    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixSingleHc595<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI
    // Common Anode, with transistors on Group pins
    SoftSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixDualHc595<SoftSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        kByteOrderGroupHighElementLow);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST
    // Common Anode, with transistors on Group pins
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixDualHc595<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        kByteOrderGroupHighElementLow);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI
    // Common Anode, with transistors on Group pins
    HardSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixDualHc595<HardSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        kByteOrderGroupHighElementLow);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
    // Common Anode, with transistors on Group pins
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixDualHc595<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        kByteOrderGroupHighElementLow);

  #else
    #error Unsupported LED_MATRIX_MODE
  #endif

  // 1-bit brightness
  ScanningModule<LedMatrix, NUM_DIGITS>
      ledModule(ledMatrix, FRAMES_PER_SECOND);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_TM1637
  #if INTERFACE_TYPE == INTERFACE_TYPE_SOFT_TMI
    using TmiInterface = SoftTmiInterface;
    TmiInterface tmiInterface(CLK_PIN, DIO_PIN, BIT_DELAY);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SOFT_TMI_FAST
    using TmiInterface = SoftTmiFastInterface<CLK_PIN, DIO_PIN, BIT_DELAY>;
    TmiInterface tmiInterface;
  #endif
  Tm1637Module<TmiInterface, NUM_DIGITS> ledModule(tmiInterface);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_MAX7219
  #if INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI_FAST
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #endif
  Max7219Module<SpiInterface, NUM_DIGITS> ledModule(
      spiInterface, kDigitRemapArray8Max7219);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595
  // Common Anode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI_FAST
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #endif

  Hc595Module<SpiInterface, NUM_DIGITS> ledModule(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderSegmentHighDigitLow,
      kDigitRemapArray8Hc595
  );

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT
  // Common Anode, with transitors on Group pins
  #if DIRECT_INTERFACE_TYPE == DIRECT_INTERFACE_TYPE_NORMAL
    DirectModule<NUM_DIGITS> ledModule(
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        SEGMENT_PINS,
        DIGIT_PINS);
  #else
    DirectFast4Module<
        8, 9, 10, 16, 14, 18, 19, 15, // segment pins
        4, 5, 6, 7, // digit pins
        NUM_DIGITS
    > ledModule(
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND);
  #endif

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HYBRID
  // Common Cathode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI_FAST
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #endif
  HybridModule<SpiInterface, NUM_DIGITS> ledModule(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_FULL
  // Common Anode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI_FAST
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #endif

  Hc595Module<SpiInterface, NUM_DIGITS> ledModule(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow,
      nullptr /*remapArray*/
  );

#else
  #error Unknown LED_DISPLAY_TYPE
#endif

LedDisplay ledDisplay(ledModule);
NumberWriter numberWriter(ledDisplay);
ClockWriter clockWriter(ledDisplay);
TemperatureWriter temperatureWriter(ledDisplay);
CharWriter charWriter(ledDisplay);
StringWriter stringWriter(charWriter);

// Setup the various resources.
void setupAceSegment() {

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
    spiInterface.begin();
  #endif

  ledMatrix.begin();
  ledModule.begin();

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_TM1637
  tmiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(2); // 1-7

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_MAX7219
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(2); // 0-15

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HYBRID
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_FULL
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#else
  #error Unknown LED_DISPLAY_TYPE

#endif
}

#if USE_INTERRUPT == 1
void renderNow() {
  ledModule.renderFieldNow();
}

void setupInterupt() {
  Timer1.initialize(ledModule.getMicrosPerField());
  Timer1.attachInterrupt(renderNow);
}
#endif

//------------------------------------------------------------------
// Configurations for AceSegmentDemo
//------------------------------------------------------------------

// State of loop, whether paused or not.
const uint8_t DEMO_LOOP_MODE_AUTO = 0;
const uint8_t DEMO_LOOP_MODE_PAUSED = 1;
uint8_t demoLoopMode = DEMO_LOOP_MODE_AUTO;

// Selection of demo.
const uint8_t DEMO_MODE_HEX_NUMBERS = 0;
const uint8_t DEMO_MODE_CLOCK = 1;
const uint8_t DEMO_MODE_UNSIGNED_DEC_NUMBERS = 2;
const uint8_t DEMO_MODE_SIGNED_DEC_NUMBERS = 3;
const uint8_t DEMO_MODE_TEMPERATURE_C = 4;
const uint8_t DEMO_MODE_TEMPERATURE_F = 5;
const uint8_t DEMO_MODE_CHAR = 6;
const uint8_t DEMO_MODE_STRINGS = 7;
const uint8_t DEMO_MODE_SCROLL = 8;
const uint8_t DEMO_MODE_SPIN = 9;
const uint8_t DEMO_MODE_SPIN_2 = 10;
const uint8_t DEMO_MODE_COUNT = 11;
uint8_t demoMode = DEMO_MODE_TEMPERATURE_C;
uint8_t prevDemoMode = demoMode - 1;

static const uint16_t DEMO_INTERNAL_DELAY[DEMO_MODE_COUNT] = {
  10, // DEMO_MODE_HEX_NUMBERS
  20, // DEMO_MODE_CLOCK
  10, // DEMO_MODE_UNSIGNED_DEC_NUMBERS
  10, // DEMO_MODE_SIGNED_DEC_NUMBERS
  100, // DEMO_MODE_TEMPERATURE_C
  100, // DEMO_MODE_TEMPERATURE_F
  200, // DEMO_MODE_CHAR
  500, // DEMO_MODE_STRINGS
  300, // DEMO_MODE_SCROLL
  100, // DEMO_MODE_SPIN
  100, // DEMO_MODE_SPIN2
};


//-----------------------------------------------------------------------------

void writeHexNumbers() {
  static uint16_t w = 0;

  numberWriter.writeHexWordAt(0, w);
  w++;
}

//-----------------------------------------------------------------------------

void writeUnsignedDecNumbers() {
  static uint16_t w = 0;

  uint8_t written = numberWriter.writeUnsignedDecimalAt(0, w, -3);
  numberWriter.clearToEnd(written);
  incrementMod(w, (uint16_t) 2000);
}

//-----------------------------------------------------------------------------

void writeSignedDecNumbers() {
  static int16_t w = -999;

  numberWriter.writeSignedDecimalAt(0, w, 4);
  w++;
  if (w > 999) w = -999;
}

//-----------------------------------------------------------------------------

void writeTemperatureC() {
  static int16_t w = -9;

  temperatureWriter.writeTempDegCAt(0, w, 4);
  w++;
  if (w > 99) w = -9;
}

//-----------------------------------------------------------------------------

void writeTemperatureF() {
  static int16_t w = -9;

  temperatureWriter.writeTempDegFAt(0, w, 4);
  w++;
  if (w > 99) w = -9;
}

//-----------------------------------------------------------------------------

void writeClock() {
  static uint8_t hh = 0;
  static uint8_t mm = 0;

  clockWriter.writeHourMinute(hh, mm);

  incrementMod(mm, (uint8_t)60);
  if (mm == 0) {
    incrementMod(hh, (uint8_t)60);
  }
}

//-----------------------------------------------------------------------------

void writeChars() {
  static uint8_t b = 0;

  numberWriter.writeHexByteAt(0, b);
  charWriter.writeCharAt(2, '-');
  charWriter.writeCharAt(3, b);

  uint8_t numChars = charWriter.getNumChars();
  if (numChars == 0) { // 0 means 256
    b++;
  } else {
    incrementMod(b, numChars);
  }
}

//-----------------------------------------------------------------------------

void writeStrings() {
  // These are defined in the function because the F() works only inside
  // functions.
  static const __FlashStringHelper* STRINGS[] = {
    F("0123"),
    F("0.123"),
    F("0.1 "),
    F("0.1.2.3."),
    F("a.b.c.d"),
    F(".1.2.3"),
    F("brian"),
  };
  static const uint8_t NUM_STRINGS = sizeof(STRINGS) / sizeof(STRINGS[0]);

  static uint8_t i = 0;

  uint8_t written = stringWriter.writeStringAt(0, STRINGS[i]);
  stringWriter.clearToEnd(written);

  incrementMod(i, NUM_STRINGS);
}

//-----------------------------------------------------------------------------

StringScroller stringScroller(ledDisplay);

static const char SCROLL_STRING[] PROGMEM =
"the quick brown fox jumps over the lazy dog, "
"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG. "
"[0123456789]";

void scrollString() {
  static bool isInit = false;
  static bool scrollLeft = true;

  if (! isInit) {
    if (scrollLeft) {
      stringScroller.initScrollLeft(
          (const __FlashStringHelper*) SCROLL_STRING);
    } else {
      stringScroller.initScrollRight(
          (const __FlashStringHelper*) SCROLL_STRING);
    }
    isInit = true;
  }

  bool isDone;
  if (scrollLeft) {
    isDone = stringScroller.scrollLeft();
  } else {
    isDone = stringScroller.scrollRight();
  }

  if (isDone) {
    scrollLeft = !scrollLeft;
    isInit = false;
  }
}

//-----------------------------------------------------------------------------

static const uint8_t NUM_SPIN_PATTERNS = 3;

const uint8_t SPIN_PATTERNS[NUM_SPIN_PATTERNS][4] PROGMEM = {
  { 0x10, 0x01, 0x08, 0x02 },  // Frame 0
  { 0x20, 0x08, 0x01, 0x04 },  // Frame 1
  { 0x09, 0x00, 0x00, 0x09 },  // Frame 2
};

void spinDisplay() {
  static uint8_t i = 0;
  const uint8_t* patterns = SPIN_PATTERNS[i];
  ledDisplay.writePatternsAt_P(0, patterns, 4);

  incrementMod(i, NUM_SPIN_PATTERNS);
}

//-----------------------------------------------------------------------------

static const uint8_t NUM_SPIN_PATTERNS_2 = 6;

const uint8_t SPIN_PATTERNS_2[NUM_SPIN_PATTERNS_2][4] PROGMEM = {
  { 0x03, 0x03, 0x03, 0x03 },  // Frame 0
  { 0x06, 0x06, 0x06, 0x06 },  // Frame 1
  { 0x0c, 0x0c, 0x0c, 0x0c },  // Frame 2
  { 0x18, 0x18, 0x18, 0x18 },  // Frame 3
  { 0x30, 0x30, 0x30, 0x30 },  // Frame 4
  { 0x21, 0x21, 0x21, 0x21 },  // Frame 5
};

void spinDisplay2() {
  static uint8_t i = 0;
  const uint8_t* patterns = SPIN_PATTERNS_2[i];
  ledDisplay.writePatternsAt_P(0, patterns, 4 /*len*/);

  incrementMod(i, NUM_SPIN_PATTERNS_2);
}

//-----------------------------------------------------------------------------

/** Display the demo pattern selected by demoMode. */
void updateDemo() {
  if (demoMode == DEMO_MODE_HEX_NUMBERS) {
    writeHexNumbers();
  } else if (demoMode == DEMO_MODE_CLOCK) {
    writeClock();
  } else if (demoMode == DEMO_MODE_UNSIGNED_DEC_NUMBERS) {
    writeUnsignedDecNumbers();
  } else if (demoMode == DEMO_MODE_SIGNED_DEC_NUMBERS) {
    writeSignedDecNumbers();
  } else if (demoMode == DEMO_MODE_TEMPERATURE_C) {
    writeTemperatureC();
  } else if (demoMode == DEMO_MODE_TEMPERATURE_F) {
    writeTemperatureF();
  } else if (demoMode == DEMO_MODE_CHAR) {
    writeChars();
  } else if (demoMode == DEMO_MODE_STRINGS) {
    writeStrings();
  } else if (demoMode == DEMO_MODE_SCROLL) {
    scrollString();
  } else if (demoMode == DEMO_MODE_SPIN) {
    spinDisplay();
  } else if (demoMode == DEMO_MODE_SPIN_2) {
    spinDisplay2();
  }
}

/** Go to the next demo */
void nextDemo() {
  prevDemoMode = demoMode;
  incrementMod(demoMode, DEMO_MODE_COUNT);
  ledDisplay.clear();

  updateDemo();
}

/** Loop within a single demo. */
void demoLoop() {
  static uint16_t lastUpdateMillis = millis();

  uint16_t demoInternalDelay = DEMO_INTERNAL_DELAY[demoMode];
  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - lastUpdateMillis) >= demoInternalDelay) {
    lastUpdateMillis = nowMillis;
    if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
      updateDemo();
    }
  }
}

void renderField() {
  #if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING \
      || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT \
      || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HYBRID \
      || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_FULL \
      || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595
    ledModule.renderFieldWhenReady();
  #else
    #if TM_FLUSH_METHOD == TM_FLUSH_METHOD_NORMAL
      ledModule.flush();
    #elif TM_FLUSH_METHOD == TM_FLUSH_METHOD_INCREMENTAL
      ledModule.flushIncremental();
    #endif
  #endif
}

void singleStep() {
  renderField();
}

//------------------------------------------------------------------
// Configurations for AceButton
//------------------------------------------------------------------

const uint8_t RENDER_MODE_AUTO = 0;
const uint8_t RENDER_MODE_PAUSED = 1;
uint8_t renderMode = RENDER_MODE_AUTO;

// Configuration for AceButton, to support Single-Step

#if BUTTON_TYPE == BUTTON_TYPE_DIGITAL

  ButtonConfig buttonConfig;
  AceButton modeButton(&buttonConfig, MODE_BUTTON_PIN);
  AceButton changeButton(&buttonConfig, CHANGE_BUTTON_PIN);

#elif BUTTON_TYPE == BUTTON_TYPE_ANALOG

  AceButton modeButton((uint8_t) MODE_BUTTON_PIN);
  AceButton changeButton((uint8_t) CHANGE_BUTTON_PIN);
  AceButton* const BUTTONS[] = {&modeButton, &changeButton};
  const uint16_t LEVELS[] = ANALOG_BUTTON_LEVELS;

  LadderButtonConfig buttonConfig(
      ANALOG_BUTTON_PIN,
      sizeof(LEVELS) / sizeof(LEVELS[0]),
      LEVELS,
      sizeof(BUTTONS) / sizeof(BUTTONS[0]),
      BUTTONS
  );

#else

  #error Unknown BUTTON_TYPE

#endif

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  (void) buttonState;

  uint8_t pin = button->getPin();
  if (pin == MODE_BUTTON_PIN) {
    switch (eventType) {
      case AceButton::kEventReleased:
      case AceButton::kEventClicked:
        if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
          demoLoopMode = DEMO_LOOP_MODE_PAUSED;
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo loop paused"));
          }
        } else if (demoLoopMode == DEMO_LOOP_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo stepped"));
          }
          updateDemo();
        }
        break;

      case AceButton::kEventLongPressed:
        if (demoLoopMode == DEMO_LOOP_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo loop enabled"));
          }
          demoLoopMode = DEMO_LOOP_MODE_AUTO;
        }
        break;

      case AceButton::kEventDoubleClicked:
        if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): next demo"));
          }
          nextDemo();
        }
        break;
    }
  } else if (pin == CHANGE_BUTTON_PIN) {
    switch (eventType) {
      case AceButton::kEventReleased:
      case AceButton::kEventClicked:
        if (renderMode == RENDER_MODE_AUTO) {
        #if USE_INTERRUPT
          Timer1.stop();
        #endif
          renderMode = RENDER_MODE_PAUSED;
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): paused"));
          }
        } else if (renderMode == RENDER_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): stepping"));
          }
          singleStep();
        }
        break;

      case AceButton::kEventLongPressed:
        if (renderMode == RENDER_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): switching to auto rendering"));
          }
        #if USE_INTERRUPT
          Timer1.start();
        #endif
          renderMode = RENDER_MODE_AUTO;
        }
        break;
    }
  }
}

void setupAceButton() {
#if BUTTON_TYPE == BUTTON_TYPE_DIGITAL
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANGE_BUTTON_PIN, INPUT_PULLUP);
#endif

  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
}

// Check AceButtons, limiting sampling rate to about 200/seconds to avoid
// problems on ESP8266 using analogRead().
void checkButtons() {
  static uint16_t prevMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevMillis) >= 5) {
    prevMillis = nowMillis;

  #if BUTTON_TYPE == BUTTON_TYPE_DIGITAL
    modeButton.check();
    changeButton.check();
  #else
    buttonConfig.checkButtons();
  #endif
  }
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  if (ENABLE_SERIAL_DEBUG >= 1) {
    Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
    while (!Serial); // Wait until Serial is ready - Leonardo/Micro
    Serial.println(F("setup(): begin"));
  }

  setupAceButton();
  setupAceSegment();
#if USE_INTERRUPT
  setupInterupt();
#endif

  if (ENABLE_SERIAL_DEBUG >= 1) {
    Serial.println(F("setup(): end"));
  }

  updateDemo();
}

void loop() {
#if ! USE_INTERRUPT
  if (renderMode == RENDER_MODE_AUTO) {
    renderField();
  }
#endif

  if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
    demoLoop();
  }

  checkButtons();
}
