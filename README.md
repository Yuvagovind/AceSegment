# AceSegment

[![AUnit Tests](https://github.com/bxparks/AceSegment/actions/workflows/aunit_tests.yml/badge.svg)](https://github.com/bxparks/AceSegment/actions/workflows/aunit_tests.yml)

An adjustable, configurable, and extensible framework for rendering seven
segment LED displays on Arduino platforms. Supported wiring confirguations
include direct pin wiring to the microcontroller, through a 74HC595 Shift
Register that is accessed through software or hardware SPI, or using an TM1637
LED driver chip.

**Version**: 0.4+ (2021-04-17)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**Status**: Need to add documentation for `Tm1637Module`.

## Table of Contents

* [Summary](#Summary)
* [Installation](#Installation)
    * [Source Code](#SourceCode)
    * [Documentation](#Documentation)
    * [Examples](#Examples)
* [LED Wiring](#LEDWiring)
* [Usage](#Usage)
    * [Include Header and Namespace](#HeaderAndNamespace)
    * [Classes](#Classes)
    * [Setting Up the Scanning Module](#SettingUpScanningModule)
    * [Choosing the LedMatrix](#ChoosingLedMatrix)
        * [Resistors and Transistors](#ResistorsAndTransistors)
        * [Pins Wired Directly, Common Cathode](#LedMatrixDirectCommonCathode)
        * [Pins Wired Directly, Common Anode](#LedMatrixDirectCommonAnode)
        * [Segments On 74HC595 Shift Register](#LedMatrixSingleShiftRegister)
        * [Digits and Segments On Two Shift Registers](#LedMatrixDualShiftRegister)
    * [Using the ScanningModule](#UsingScanningModule)
        * [Writing the Digit Bit Patterns](#DigitBitPatterns)
        * [Global Brightness](#GlobalBrightness)
        * [Frames and Fields](#FramesAndFields)
        * [Rendering by Polling](#RenderingByPolling)
        * [Rendering using Interrupts](#RenderingUsingInterrupts)
    * [NumberWriter](#NumberWriter)
    * [ClockWriter](#ClockWriter)
    * [TemperatureWriter](#TemperatureWriter)
    * [CharWriter](#CharWriter)
    * [StringWriter](#StringWriter)
* [Advanced Usage](#AdvancedUsage)
    * [DigitalWriteFast on AVR](#DigitalWriteFast)
* [Resource Consumption](#ResourceConsumption)
* [System Requirements](#SystemRequirements)
    * [Hardware](#Hardware)
    * [Tool Chain](#ToolChain)
    * [Operating System](#OperatingSystem)
* [License](#License)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="Summary"></a>
## Summary

The AceSegment library provides a number of classes that can display
digits, characters and other patterns on an "seven segment" LED display.
The framework is intended to be used with LED displays which are
directly connected to the microcontroller through the GPIO pins,
instead of through a specialized LED display driver chip, like
the [MAX7219](https://www.maximintegrated.com/en/datasheet/index.mvp/id/1339)
or the
[MC14489B](http://cache.freescale.com/files/timing_interconnect_access/doc/inactive/MC14489B.pdf).
The framework does support the 74HC595 Shift Register chip which is a
general purpose chip that helps reduce the GPIO pin usage.

Here are the features supported by this framework:

* multiplexing of segments at a selectable frame rate
* common cathode or common anode configurations
* resistors on segments or resistors on digits
* LED display directly connected to the GPIO pins
* LED display connected through a 74HC595 shift register chip
* communication with the 74HC595 through
    * software SPI (using `shiftOut()`)
    * hardware SPI (using `<SPI.h>`)
* transistors drivers to handle high currents
* LED module using the TM1637 chip

The framework splits the responsibility of displaying LED digits into several
components:

* The `SpiInterface` is a thin wrapper around either a software SPI or hardware
  SPI.
* The `LedMatrix` knows how to enable or disable LED segments on various digit
  groups. Different subclasses the `LedMatrix` are provided to support:
    * resistors on segments
    * transistors on digits or segments
    * using 74HC595 with `shiftOut()` or hardware SPI
* The `LedModule` class and subclasses such as `ScanningModule` know how to
  render the array of digit patterns to the LED module using multiplexing.
* The `LedDisplay` knows how to write the segment bit patterns to an
  `LedModule`.

The rendering of an array of bit patterns is split into 2 parts:

* a *frame* is one complete rendering of the LED display
* a *field* is a partial rendering of a single frame

A frame rate of about 60Hz will be sufficient to prevent obvious flickering of
the LED. Depending on the configuration of the `ScanningModule` class, we could
reasonably have between 4 and 64 fields per frame (this is partially a
user-selectable parameter), giving us a fields per second rate of 240Hz to
3840Hz.

At 3840 fields per second, a single field needs to be written in less than 260
microseconds. The AceSegment library is able to meet this timing requirement
because `ScanningModule::renderFieldNow()` is able to render a single field
with a maximum CPU time of 124 microseconds on a 16MHz ATmega328P
microcontroller (Arduino UNO, Nano, Mini, etc).

<a name="Installation"></a>
## Installation

The latest stable release will eventually be available in the Arduino IDE
Library Manager. Search for "AceSegment". Click install. It is not there
yet.

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceSegment), checking out the
`develop` branch, then manually copying over the contents to the `./libraries`
directory used by the Arduino IDE. (The result is a directory named
`./libraries/AceSegment`.) The `master` branch contains the stable release.

<a name="SourceCode"></a>
### Source Code

The source files are organized as follows:
* `src/AceSegment.h` - main header file
* `src/ace_segment/` - implementation files
* `src/ace_segment/testing/` - internal testing files
* `tests/` - unit tests which require [AUnit](https://github.com/bxparks/AUnit)
* `examples/` - example sketches
* `docs/` - contains the doxygen docs and additional manual docs

<a name="Documentation"></a>
### Documentation

* this `README.md` file
* [Doxygen docs published on GitHub Pages](https://bxparks.github.io/AceSegment/html).

<a name="Examples"></a>
### Examples

The following example sketches are provided:

* [AceSegmentDemo.ino](examples/AceSegmentDemo):
  a demo program that exercises a large fraction of the feature of the framework
* [AutoBenchmark.ino](examples/AutoBenchmark):
  a program that performs CPU benchmarking of almost all of the various
  supported configurations of the framework

<a name="LEDWiring"></a>
## LED Wiring

AceSegment library supports the following wiring configurations:

* common cathode
* common anode
* resistors on segments
* resistors on digits (not supported)

The driver classes assume that the pins are connected directly to the GPIO pins
of the microcontroller. In other words, for a 4 digit x 8 segment LED display,
you would need 12 GPIO pins. This is the cheapest and simplest option if you
have enough pins available because you need nothing else because the current
limiting resistors. If the project is not able to allocate this many pins, then
the usual solution is to use a Serial to Parallel converter such as the 74HC595
chip.

At first glance, there is not an obvious difference between "resistors on
segments" configuration and "resistors on digits". I recommend using resistors
on segments if at all possible. That's because the LEDs with the resistors are
the ones that can be turned on at the same time, and the ones without the
resistors are multiplexed to give the illusion of a fully lit display. With the
resistors on the segments, all the segments on one digit can be activated at the
same time, and we can use pulse width modulation on the digit line to control
the brightness of a single digit.

Multiple LED segments will be connected to a single GPIO pin. If the total
current on the pin exceeds the rated limit, then a transistor will need to be
added to handle the current. The framework can be configured to support these
transistors.

<a name="Usage"></a>
## Usage

<a name="HeaderAndNamespace"></a>
### Include Header and Namespace

Only a single header file `AceSegment.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are defined in the `ace_segment` namespace. To use the code without
prepending the `ace_segment::` prefix, use the `using` directive:

```
#include <AceSegment.h>
using namespace ace_segment;
```

<a name="Classes"></a>
### Classes

Here are the classes in the library which will be most useful to the
end-users, listed roughly from low-level to higher-level classes which often
depend on the lower-level classes:

* SpiInterface
    * Thin-wrapper classes to indicate whether we are using software or hardware
      SPI. There are 3 implementations:
    * `SwSpiInterface`
        * Software SPI using `shiftOut()`
    * `HwSpiInterface`
        * Native hardware SPI.
    * `FastSwSpiInterface`
        * Software SPI using `digitalWriteFast()` on AVR processors
* `LedMatrix`: Various subclasses capture the wiring of the matrix of LEDs.
    * `LedMatrixDirect`
        * Group pins and element pins are directly accessed through the
          microcontroller pins.
    * `LedMatrixDirectFast`
        * Same as `LedMatrixDirect` but using `digitalWriteFast()` on AVR
          processors
    * `LedMatrixSingleShiftRegister`
        * Group pins are access directly, but element pins are access through an
          74HC595 chip through SPI using one of SpiInterface classes
    * `LedMatrixDualShiftRegister`
        * Both group and element pions are access through two 74HC595 chips
          through SPI using one of the SpiInterface classes
* `LedModule`
    * Base interface for all hardware dependent implementation of a
      seven-segment LED module.
    * `ScanningModule`
        * An implementation of `LedModule` for LED segment modules which do not
          have a hardware controller, and must be multiplexed across multiple
          digits by the host microcontroller
        * Uses the `LedMatrix` classes to send the segment patterns to the
          actual LED module
    * `Tm1637Module`
        * An implementation of `LedModule` using a TM1637 chip.
* `LedDisplay`
    * Knows how to write bit patterns to a `LedModule`.
    * Provides common interfaces and common methods to various Writer classes.
* Writers
    * Helper classes built on top of the `LedDisplay` which provide higher-level
      interface to the LED module, such as printing numbers, time (hh:mm),
      and ASCII characters
    * `NumberWriter`
        * A class that writes integers in decimal or hexadecimal format to the
          `LedDisplay`.
        * A few additional characters are supported: `kCharSpace`, `kCharMinus`
    * `ClockWriter`
        * A class that writes a clock string "hh:mm" to `LedDisplay`.
        * A few additional symbols are supported: `kCharSpace`, `kCharMinus` and
          `kPatternA` ("A" for AM) and `kPatternP` ("P" for PM).
    * `TemperatureWriter`
        * A class that writes temperatures with a degrees symbol or optionally
          with "C" or "F" symbol.
    * `CharWriter`
        * A class that convert an ASCII character represented by a `char` (code
          0-127) to a bit pattern used by `SegmentDriver` class.
        * Not all ASCII characters can be rendered on a seven segment display
          legibly but the `CharWriter` tries its best.
    * `StringWriter`
        * A class that prints strings of `char` to a `CharWriter`, which in
          turns, prints to the `LedDisplay`.
        * It tries to be smart about collapsing decimal point `.` characters
          into the native decimal point on a seven segment LED display.

The dependency diagram among these classes looks something like this:

```
   StringWriter    ClockWriter  TemperatureWriter
        |              \           /
        V               v         v
     CharWriter         NumberWriter
             \            /
              v          v
               LedDisplay
                   |            (hardware independent)
-------------------|-------------------------------------
                   |            (hardware dependent)
                   v
                LedModule
                   ^
                   |
          +--------+--------+
          |                 |
   ScanningModule       Tm1637Module
                            |
                            v
                        SwWireInterface
                        FastSwWireInterface


                 ScanningModule
                 /      |     \
                /       |      .-------------.
               v        v                     v
  LedMatrixDirect   LedMatrixSingleSftRgstr  LedMatrixDualShiftRegister
LedMatrixDirectFast               \             /
                                   \           /
                                    v         v
                                   SwSpiInterface
                                   HwSpiInterface
                                   FastSwSpiInterface
```

<a name="SettingUpScanningModule"></a>
### Setting Up the Scanning Module

A series of resources must be built up to finally create an instance of
`ScanningModule`. For an LED module using direct scanning, the `ScanningModule`
is used, and the resource creation occurs in roughly 5 stages, with the objects
in the later stages depending on the objects created in the earlier stage:

1. The SpiInterface object determines whether software SPI or hardware SPI is
   used. Needed only by `LedMatrixSingleShiftRegister` and
   `LedMatrixDualShiftRegister` classes.
1. The LedMatrix object determine how the LEDs are wired and how to
   communicate to the LED segments.
1. The `ScanningModule` represents the actual LED segment module using
   direct scanning, or scanning through an 74HC595 shift register chip.
1. The `LedDisplay` writes seven-segment bit patterns to the underlying
   `LedModule`.
1. The various Writer classes which translate higher level characters and
   strings into bit patterns used by `LedDisplay`.

A typical resource creation code looks like this:

```C++
const uint8_t NUM_DIGITS = 4;
const uint8_t NUM_SEGMENTS = 8;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 11, 12, 13, 14, 15};
const uint16_t FRAMES_PER_SECOND = 60;

// The chain of resources.
using LedMatrix = LedMatrixDirect<>;
LedMatrix ledMatrix(
    LedMatrix::kActiveLowPattern /*groupOnPattern*/,
    LedMatrix::kActiveLowPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS,
    NUM_SEGMENTS,
    SEGMENT_PINS);
ScanningModule<LedMatrix, NUM_DIGITS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
LedDisplay ledDisplay(scanningModule);

NumberWriter hexWriter(ledDisplay);
ClockWriter clockWriter(ledDisplay);
CharWriter charWriter(ledDisplay);
StringWriter stringWriter(charWriter);
...

void setupAceSegment() {
  ledMatrix.begin();
  scanningModule.begin();
}

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

  ...
  setupAceSegment();

  Serial.println(F("setup(): end"));
}
```

<a name="ChoosingLedMatrix"></a>
### Choosing the LedMatrix

The `LedMatrix` captures the wiring information about the LED module.
The best way to show how to use these methods is probably through
examples.

<a name="ResistorsAndTransistors"></a>
#### Resistors and Transistors

In the following circuit diagrams, `R` represents the current limiting resistors
and `T` represents the driving transistor. The resistor protects each LED
segment, and the transistor prevents overloading the GPIO pin of the
microcontroller.

On an Arduino Nano for example, each GPIO pin can handle 40mA of current. If the
value of `R` is high enough that each segment only drew only 5mA of current, the
transistor could be omitted since 8 x 5mA is 40mA, which is within the limit of
a single GPIO pin.

If the `R` value is increased so that each LED segment is pushed to its rated
current limit (probably 10-15mA), then 8 segments could potentially push
80-120mA into the single digit pin of the Nano, which would exceed the maximum
rating. A driver transistor would be needed on the digit pin to handle this
current.

If driver transistors are used on the digits, then it is likely that the logic
levels need to be inverted software. In the examnple below, using common cathode
display, with the resistors on the segments, an NPN transistor on the digit,
the digit pin on the microcontroll (D12) needs to be HIGH to turn on the LED
segement. In contrast, if the D12 pin was connected directly to the LED, the
digit pin would need to be set LOW to turn on the LED.

```
MCU                      LED display (Common Cathode)
+-----+                  +------------------------+
|  D08|------ R ---------|a -------.              |
|  D09|------ R ---------|b -------|--------.     |
|  D10|------ R ---------|c        |        |     |
|  D11|------ R ---------|d      -----    -----   |
|  D12|------ R ---------|e       \ /      \ /    |
|  D13|------ R ---------|f      --v--    --v--   |
|  D14|------ R ---------|g        |        |     |
|  D15|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|     |              +---|D1 ------+--------'     |
|     |             /    |                        |
|     |            /     +------------------------+
|  D04|----- R ---| NPN
|     |            \
+-----+             v
                    |
                   GND
```

If the LED display was using common anode, instead of common cathode as shown
above, then a PNP transistor would be used, with the emitter tied to Vcc. Again,
the logic level on the D12 pin will become inverted compared to wiring the digit
pin directly to the microcontroller.

<a name="LedMatrixDirectCommonCathode"></a>
#### Pins Wired Directly, Common Cathode

The wiring for this configuration looks like this:
```
MCU                      LED display (Common Cathode)
+-----+                  +------------------------+
|  D08|------ R ---------|a -------.              |
|  D09|------ R ---------|b -------|--------.     |
|  D10|------ R ---------|c        |        |     |
|  D11|------ R ---------|d      -----    -----   |
|  D12|------ R ---------|e       \ /      \ /    |
|  D13|------ R ---------|f      --v--    --v--   |
|  D14|------ R ---------|g        |        |     |
|  D15|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|  D04|---- R - T -------|D1 ------'--------'     |
|  D05|---- R - T -------|D2                      |
|  D06|---- R - T -------|D3                      |
|  D07|---- R - T -------|D4                      |
+-----+                  +------------------------+
```

The `LedMatrixDirect` constructor is:

```C++
const uint8_t NUM_DIGITS = 4;
const uint8_t NUM_SEGMENTS = 8;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 11, 12, 13, 14, 15};
const uint16_t FRAMES_PER_SECOND = 60;

using LedMatrix = LedMatrixDirect<>;
LedMatrix ledMatrix(
    LedMatrix::kActiveHighPattern /*groupOnPattern*/,
    LedMatrix::kActiveHighPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS,
    NUM_SEGMENTS,
    SEGMENT_PINS);
ScanningModule<LedMatrix, NUM_DIGITS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
LedDisplay ledDisplay(scanningModule);
...

void setupScanningModule() {
  ledMatrix.begin();
  scanningModule.begin();
}
```

<a name="LedMatrixDirectCommonAnode"></a>
#### Pins Wired Directly, Common Anode

The wiring for this configuration looks like this:
```
MCU                      LED display (Common Anode)
+-----+                  +------------------------+
|  D08|------ R ---------|a -------.              |
|  D09|------ R ---------|b -------|--------.     |
|  D10|------ R ---------|c        |        |     |
|  D11|------ R ---------|d      --^--    --^--   |
|  D12|------ R ---------|e       / \      / \    |
|  D13|------ R ---------|f      -----    -----   |
|  D14|------ R ---------|g        |        |     |
|  D15|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|  D04|---- R - T -------|D1 ------'--------'     |
|  D05|---- R - T -------|D2                      |
|  D06|---- R - T -------|D3                      |
|  D07|---- R - T -------|D4                      |
+-----+                  +------------------------+
```

The `LedMatrixDirect` configuration is *exactly* the same as the Common Cathode
case above, except that `kActiveHighPattern` is replaced with
`kActiveLowPattern`.

<a name="LedMatrixSingleShiftRegister"></a>
#### Segments on 74HC595 Shift Register

The segment pins can be placed on a 74HC595 shift register chip that can be
accessed through SPI. The caveat is that this chip can supply only 6 mA per pin
(as opposed to 40 mA for an Arduino Nano for example). But it may be enough in
some applications.

We assume here that the registors are on the segment pins, and that we are using
a common cathode LED display.

```
MCU          74HC595             LED display (Common Cathode)
+--------+   +---------+         +------------------------+
|        |   |       Q0|--- R ---|a -------.              |
|        |   |       Q1|--- R ---|b -------|--------.     |
|     D10|---|ST_CP  Q2|--- R ---|c        |        |     |
|MOSI/D11|---|DS     Q3|--- R ---|d      -----    -----   |
| SCK/D13|---|SH_CP  Q4|--- R ---|e       \ /      \ /    |
|        |   |       Q5|--- R ---|f      --v--    --v--   |
|        |   |       Q6|--- R ---|g        |        |     |
|        |   |       Q7|--- R ---|h        |        |     |
|        |   +---------+         |         |        |     |
|        |                       |         |        |     |
|     D04|------ R - T ----------|D1 ------'--------'     |
|     D05|------ R - T ----------|D2                      |
|     D06|------ R - T ----------|D3                      |
|     D07|------ R - T ----------|D4                      |
+--------+                       +------------------------+
```

The `LedMatrixSingleShiftRegister` configuration using software SPI is:

```C++
const uint8_t NUM_DIGITS = 4;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = 11; // DS on 74HC595
const uint8_t CLOCK_PIN = 13; // SH_CP on 74HC595
const uint16_t FRAMES_PER_SECOND = 60;

// Common Cathode, with transistors on Group pins
SwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
using LedMatrix = LedMatrixSingleShiftRegister<SwSpiInterface>;
LedMatrix ledMatrix(
    spiInterface,
    LedMatrix::kActiveHighPattern /*groupOnPattern*/,
    LedMatrix::kActiveHighPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS):
ScanningModule<LedMatrix, NUM_DIGITS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
LedDisplay ledDisplay(scanningModule);
...

void setupScanningModule() {
  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
}
```

The `LedMatrixSingleShiftRegister` configuration using hardware SPI is *exactly*
the same as above but with `HwSpiInterface` replacing `SwSpiInterface`.

<a name="LedMatrixDualShiftRegister"></a>
#### Digits and Segments on Two Shift Registers

In this wiring, both the segment pins and the digit pins are wired to
two 74HC595 chips so that both sets of pins are set through SPI.

```
MCU                 74HC595             LED display (Common Cathode)
+--------+          +---------+         +------------------------+
|        |          |       Q0|--- R ---|a -------.              |
|        |          |       Q1|--- R ---|b -------|--------.     |
|     D10|--+-------|ST_CP  Q2|--- R ---|c        |        |     |
|MOSI/D11|--|-------|DS     Q3|--- R ---|d      -----    -----   |
| SCK/D13|--|---+---|SH_CP  Q4|--- R ---|e       \ /      \ /    |
|        |  |   |   |       Q5|--- R ---|f      --v--    --v--   |
|        |  |   |   |       Q6|--- R ---|g        |        |     |
|        |  |   | +-|Q7'    Q7|--- R ---|h        |        |     |
|        |  |   | | +---------+         |         |        |     |
|        |  |   | |                     |         |        |     |
|        |  |   | | 74HC595             |         |        |     |
|        |  |   | | +---------+         |         |        |     |
|        |  |   | | |       Q0|- R - T -|D1 ------+--------+     |
|        |  |   | | |       Q1|- R - T -|D2                      |
|        |  +---|-|-|ST_CP  Q2|- R - T -|D3                      |
|        |      | +-|DS     Q3|- R - T -|D4                      |
|        |      +---|SH_CP  Q4|         |                        |
|        |          |       Q5|         |                        |
|        |          |       Q6|         |                        |
+--------+          |       Q7|         +------------------------+
                    +---------+
```

The `LedMatrixDualShiftRegister` configuration is the following. Let's use
`HwSpiInterface` this time:

```C++
const uint8_t NUM_DIGITS = 4;
const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = 11; // DS on 74HC595
const uint8_t CLOCK_PIN = 13; // SH_CP on 74HC595
const uint16_t FRAMES_PER_SECOND = 60;

HwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
using LedMatrix = LedMatrixSingleShiftRegister<HwSpiInterface>;
LedMatrix ledMatrix(
    spiInterface,
    LedMatrix::kActiveHighPattern /*groupOnPattern*/,
    LedMatrix::kActiveHighPattern /*elementOnPattern*/);
ScanningModule<LedMatrix, NUM_DIGITS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
LedDisplay ledDisplay(scanningModule);
...

void setupScanningModule() {
  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
}
```

<a name="UsingScanningModule"></a>
### Using the ScanningModule

<a name="DigitBitPatterns"></a>
#### Writing Digit Bit Patterns

The `ScanningModule` contains a number of methods to write the bit patterns of
the seven segment display:
* `void writePatternAt(uint8_t pos, uint8_t pattern)`
* `void writeDecimalPointAt(uint8_t pos, bool state = true)`
* `void setBrightnessAt(uint8_t pos, uint8_t brightness)`

The `pos` is the index into the LED digit array, from `0` to `NUM_DIGITS-1`
where `0` represents the left-most digit. The `pattern` is an 8-bit integer
which maps to the LED segments using the usual convention for a seven-segment
LED ('a' is the least significant bit 0, decimal point 'dp' is the most
seignificant bit 7):
```
7-segment map:
      aaa       000
     f   b     5   1
     f   b     5   1
      ggg       666
     e   c     4   2
     e   c     4   2
      ddd  dp   333  77

Segment: dp g f e d c b a
   Bits: 7  6 5 4 3 2 1 0
```
(Sometimes, the decimal point `dp` is labeled as an `h`).

The `writeDecimalPointAt()` is a special method that sets the bit corresponding
to the decimal point ('h', bit 7), no matter what previous pattern was there in
initially. The `state` variable controls whether the decimal point should
be turned on (default) or off (false).

The `brightness` is an integer constant (0-255) associated with the digit. It
requires the `ScanningModule` object to be configured to support PWM on the
digit pins. Otherwise, the brightness is ignored.

<a name="GlobalBrightness"></a>
#### Global Brightness

If the `ScanningModule` supports it, we can control the global brightness of
the entire LED display using:

```
scanningModule.setBrightness(value);
```

Note that the `value` is an integer from `[0, NUM_DIGITS]`, and represents the
brightness of the display, where 0 means OFF and `NUM_DIGITS` means 100% ON.

The global brightness is enabled only if the `NUM_SUBFIELDS` template parameter
of the `ScanningModule` was set to be `> 1`. By default, this is set to 1. For
example, this creates a `ScanningModule` using hardware SPI, setting the
`NUM_SUBFIELDS` to be 16:

```
const uint8_t NUM_DIGITS = 4;
const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = 11; // DS on 74HC595
const uint8_t CLOCK_PIN = 13; // SH_CP on 74HC595
const uint16_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;

HwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
using LedMatrix = LedMatrixSingleShiftRegister<HwSpiInterface>;
LedMatrix ledMatrix(
    spiInterface,
    LedMatrix::kActiveHighPattern /*groupOnPattern*/,
    LedMatrix::kActiveHighPattern /*elementOnPattern*/);
ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
LedDisplay ledDisplay(scanningModule);
```

Each digit is rendered 16 times within a single field, and modulated using pulse
width modulation to control the width of that signal. The given digit will be
on only a fraction of the full interval of the entire rendering of the  field
and will appear dimmer to the human eye.

<a name="FramesAndFields"></a>
#### Frames and Fields

To understand how to the `ScanningModule` supports brightness, we first need to
explain a couple of terms that we borrowed from the field of
[video processing](https://en.wikipedia.org/wiki/Field_(video)):

* **Frame**: A frame is a complete rendering of all digits of the seven segment
  display. A frame is intended to be a single, conceptually static image of the
  LED display. Any changes in bit patterns or brightness of the digits happens
  through the rendering of multiple frames.
* **Field**: A field is a partial rendering of a frame. If the current limiting
  resistors are on the segments (recommended), then the `ScanningModule`
  multiplexes through the digits. Each rendering of the digit is a *field* and
  for a 4-digit display, there are 4 fields per frame.

A *frame* rate of about 60Hz is recommended to eliminate obvious visual
flickering. If the LED display has 4 digits, and we use "resistors on segments"
configuration, then we need to have a *field* rate of 240Hz. We will see later
that if we want dimmable digits using PWM, then we need about 8-16 subfields
within a field, giving a total *field* rate of about 2000-4000Hz. That's abaout
250-500 microseconds per field, which is surprisingly doable using an 8-bit
processor like an Arduino UNO or Nano on an ATmega328 running at 16MHz.

The primary unit of rendering in `ScanningModule` is a single field,
implemented in `ScanningModule::renderFieldNow()`. The `ScanningModule` class
keeps track of the current digit, the current frame, and the current field, and
each successive call to `renderFieldNow()` sends the appropriate bit pattern to
the LED module.

For a given requested frame rate, are 2 ways to render the fields at the correct
time, and they are explained below:

* Polling
* Interrupts

<a name="RenderingByPolling"></a>
#### Rendering By Polling

The `ScanningModule::renderFieldWhenReady()` is meant to be called at a
frequency somewhat higher than that needed to sustain the actual frame rate. It
keeps an internal variable containing the time (in `micros()`) of the previous
rendering of the field. When the time is up, it calls `renderFieldNow()` and
updates the internal clock to be ready for the next call.

The code looks like this:

```
void loop() {
  scanningModule.renderFieldWhenReady();
}
```

The problem with using this method is that it's difficult to get much else done
in the `loop()` method. If those other side things take up too much time, then
the refreshing rate of the LED module will wander, and the eyes will notice
flickering of the LED module.

Using this polling method is the easiest way to get AceSegment working. But many
non-trivial project will want to use the timer interrupt method to avoid the
flickering problem.

<a name="RenderingUsingInterrupts"></a>
#### Rendering Using Interrupts

The calling code sets up an interrupt service routine (ISR) which calls
`ScanningModule::renderFieldNow()` at exactly the periodic frequency needed to
achieve the desired frames per second and fields per second.

Unfortunately, timer interrupts are not part of the Arduino API (probably
because every microcontroller handles interrupts in a slightly different way).
For example, an ATmega328 (e.g. Arduino UNO, Nano, Mini), using an 8-bit timer
on Timer 2 looks like this:
```
ISR(TIMER2_COMPA_vect) {
  scanningModule.renderFieldNow();
}

void setup() {
  ...
  // set up Timer 2
  uint8_t timerCompareValue =
      (unsigned long) F_CPU / 1024 / scanningModul.getFieldsPerSecond() - 1;
  noInterrupts();
  TCNT2  = 0;	// Initialize counter value to 0
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= bit(WGM21); // CTC
  TCCR2B |= bit(CS22) | bit(CS21) | bit(CS20); // prescale 1024
  TIMSK2 |= bit(OCIE2A); // interrupt on Compare A Match
  OCR2A =  timerCompareValue;
  interrupts();
  ...
}

void loop() {
 ...do other stuff here...
}
```

<a name="NumberWriter"></a>
### NumberWriter

While it is exciting to be able to write any bit patterns to the LED display, we
often want to just write numbers to the LED display. The `NumberWriter` can
print integers to the `ScanningModule` using decimal or hexadecimal formats. On
platforms that support it (ATmega and ESP8266), the bit mapping table is stored
in flash memory to conserve static memory.

The class supports the following methods:

* `LedDisplay& display()`
* `void writeHexCharAt(uint8_t pos, hexchar_t c)`
* `void writeHexByteAt(uint8_t pos, uint8_t b)`
* `void writeHexWordAt(uint8_t pos, uint16_t w)`
* `void writeUnsignedDecimalAt(uint8_t pos, uint16_t num, int8_t boxSize = 0)`
* `void writeSignedDecimalAt(uint8_t pos, int16_t num, int8_t boxSize = 0)`

The `hexchar_t` type semantically represents the character set supported by this
class. It is implemented as an alias for `uint8_t`, which unfortunately means
that the C++ compiler will not warn about mixing this type with another
`uint8_t`. The range of this character set is from `[0,15]` plus 2 additional
symbols, so `[0,17]`:

* `NumberWriter::kCharSpace`
* `NumberWriter::kCharMinus`

A `NumberWriter` consumes about 150 bytes of flash memory on an AVR.

<a name="ClockWriter"></a>
### ClockWriter

There are special, 4 digit,  seven segment LED displays which replace the
decimal point with the colon symbol ":" between the 2 digits on either side so
that it can display a time in the format "hh:mm".

The class supports the following methods:

* `LedDisplay& display()`
* `void writeCharAt(uint8_t pos, hexchar_t c)`
* `void writeBcd2At(uint8_t pos, uint8_t bcd);
* `void writeDec2At(uint8_t pos, uint8_t d);
* `void writeDec4At(uint8_t pos, uint16_t dd);
* `void writeHourMinute(uint8_t hh, uint8_t mm)`
* `void writeColon(bool state = true)`

A `ClockWriter` consumes about 250 bytes of flash memory on an AVR, which
includes an instance of a `NumberWriter`.

<a name="TemperatureWriter"></a>
### TemperatureWriter

The class supports the following methods:

* `LedDisplay& display()`
* `uint8_t writeTempDegAt(uint8_t pos, int16_t temp, boxSize = 0);`
* `uint8_t writeTempDegCAt(uint8_t pos, int16_t temp, boxSize = 0);`
* `uint8_t writeTempDegFAt(uint8_t pos, int16_t temp, boxSize = 0);`

A `TemperatureWriter` consumes about 270 bytes of flash memory on an AVR, which
includes an instance of a `NumberWriter`.

<a name="CharWriter"></a>
### CharWriter

It is possible to represent many of the ASCII characters in the range `[0,127]`
on a seven-segment LED display, although some of the characters will necessarily
be crude given the limited number of segments. The `CharWriter` contains a
[mapping of ASCII](https://github.com/dmadison/LED-Segment-ASCII) characters
to seven-segment bit patterns. On platforms that support it (ATmega and
ESP8266), the bit pattern array is stored in flash memory to conserve static
memory.

The class supports the following methods:

* `LedDisplay& display()`
* `void writeCharAt(uint8_t pos, char c)`

A `CharWriter` consumes about 250 bytes of flash memory on an AVR.

<a name="StringWriter"></a>
### StringWriter

A `StringWriter` is a class that builds on top of the `CharWriter`. It knows how
to write entirely strings into the LED display. It provides the following
methods:

* `LedDisplay& display()`
* `void writeStringAt(uint8_t pos, const char* s, bool padRight = false)`

The implementation of this method is straightforward except for the handling of
a decimal point. A seven segment LED digit contains a small LED for the decimal
point. Instead of taking up an entire digit for a single '.' character, we can
collapse the '.' character into the decimal point indicator of the previous
character on the left.

The `padRight` flag tells the method to pad spaces to the right if we run out of
characters before getting to the end of the digits on the LED display.

Scrolling can be achieved by writing success string fragments into digit 0, with
a scrolling timing interval:
```
void scrollString(const char* s) {
  static uint8_t i = 0;

  if (i >= strlen(s)) i = 0;
  stringWriter.writeStringAt(0, &s[i], true /* padRight */);
  i++;
}
```

A `StringWriter` consumes about 400 bytes of flash memory on an AVR, which
includes an instance of `CharWriter`.

(TODO: Maybe merge `StringWriter` and `CharWriter`?)

<a name="AdvancedUsage"></a>
## Advanced Usage

<a name="DigitalWriteFast"></a>
### DigitalWriteFast on AVR Processors

On the AVR processors (e.g. Arduino Nano, SparkFun Pro Micro), the default
`digitalWrite()`, `digitalRead()` and `pinMode()` functions can be significantly
improved (up to 50X performance, and a lot less flash memory) if the pin number
and output value are known at compile-time. There are at least 2 libraries that
provide the `digitalWriteFast()` variants:

* https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast, or
* https://github.com/NicksonYap/digitalWriteFast

I have written versions of some lower-level classes to take advantage of
`digitalWriteFast()`:

* `scanning/LedMatrixDirectFast.h`
    * Variant of `LedMatrixDirect` using `digitalWriteFast()`
* `hw/FastSwSpiInterface.h`
    * Variant of `SwSpiInterface.h` using  `digitalWriteFast()`
* `hw/FastSwWireInterface.h`
    * Variant of `SwWireInterface.h` using `digitalWriteFast()`

Since these header files require an external `digitalWriteFast` library to be
installed, and they are only valid for AVR processors, these header files are
*not* included in the master `<AceSegment.h>` file. If you want to use them, you
need to include these headers manually, like this:

```C++
#include <AceSegment.h> // do this first
#include <digitalWriteFast.h> // from 3rd party library
#include <ace_segment/hw/FastSwSpiInterface.h>
#include <ace_segment/hw/FastSwWireInterface.h>
#include <ace_segment/scanning/LedMatrixDirectFast.h>
```

<a name="ResourceConsumption"></a>
## Resource Consumption

### Static Memory

Here are the sizes of the various classes on the 8-bit AVR microcontrollers
(Arduino Uno, Nano, etc):

* sizeof(SwSpiInterface): 3
* sizeof(FastSwSpiInterface<1,2,3>): 1
* sizeof(HwSpiInterface): 3
* sizeof(LedMatrixDirect<>): 11
* sizeof(LedMatrixDirectFast<0..3, 0..7>): 3
* sizeof(LedMatrixSingleShiftRegister<SwSpiInterface>): 10
* sizeof(LedMatrixDualShiftRegister<HwSpiInterface>): 5
* sizeof(LedDisplay): 3
* sizeof(ScanningModule<LedMatrixBase, 4, 1>): 25
* sizeof(Tm1637Display<SwWireInterface, 4>): 12
* sizeof(NumberWriter): 2
* sizeof(ClockWriter): 3
* sizeof(CharWriter): 2
* sizeof(StringWriter): 2

### Flash Memory

For the most part, the user pays only for the feature that is being used. For
example, if the `CharWriter` (which consumes 312 bytes of flash) is not used, it
is not loaded into the program.

The full details are given in
[examples/MemoryBenchmark](examples/MemoryBenchmark). Here are the flash and
static memory consumptions for various configurations on an Arduino Nano
(ATmega328):

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    456/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| ScanningModule(direct)          |   1700/   76 |  1244/   65 |
| ScanningModule(single_sw_spi)   |   1724/   70 |  1268/   59 |
| ScanningModule(single_hw_spi)   |   1786/   71 |  1330/   60 |
| ScanningModule(dual_sw_spi)     |   1626/   61 |  1170/   50 |
| ScanningModule(dual_hw_spi)     |   1700/   62 |  1244/   51 |
|---------------------------------+--------------+-------------|
| ScanningModule(direct_fast)     |   1460/  104 |  1004/   93 |
| ScanningModule(single_sw_fast)  |   1616/   68 |  1160/   57 |
| ScanningModule(dual_sw_fast)    |   1226/   59 |   770/   48 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           |   1814/   43 |  1358/   32 |
| Tm1637Display(Fast)             |   1152/   40 |   696/   29 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |    538/   11 |    82/    0 |
| NumberWriter+Stub               |    692/   32 |   236/   21 |
| ClockWriter+Stub                |    792/   33 |   336/   22 |
| CharWriter+Stub                 |    792/   32 |   336/   21 |
| StringWriter+Stub               |    940/   40 |   484/   29 |
+--------------------------------------------------------------+
```

And here are the memory consumption numbers for an ESP8266:

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 256700/26784 |     0/    0 |
|---------------------------------+--------------+-------------|
| ScanningModule(direct)          | 257972/26860 |  1272/   76 |
| ScanningModule(single_sw_spi)   | 258044/26868 |  1344/   84 |
| ScanningModule(single_hw_spi)   | 259148/26876 |  2448/   92 |
| ScanningModule(dual_sw_spi)     | 257928/26848 |  1228/   64 |
| ScanningModule(dual_hw_spi)     | 259128/26856 |  2428/   72 |
|---------------------------------+--------------+-------------|
| ScanningModule(direct_fast)     |     -1/   -1 |    -1/   -1 |
| ScanningModule(single_sw_fast)  |     -1/   -1 |    -1/   -1 |
| ScanningModule(dual_sw_fast)    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           | 258168/26816 |  1468/   32 |
| Tm1637Display(Fast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubDisplay                     | 256884/26792 |   184/    8 |
| NumberWriter+Stub               | 257380/26792 |   680/    8 |
| ClockWriter+Stub                | 257140/26800 |   440/   16 |
| CharWriter+Stub                 | 257092/26792 |   392/    8 |
| StringWriter+Stub               | 257292/26816 |   592/   32 |
+--------------------------------------------------------------+
```

### CPU Cycles

The CPU benchmark numbers can be seen in
[examples/AutoBenchmark](examples/AutoBenchmark).

Here are the CPU numbers for an AVR processor:

```
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |    72/   76/   88 |     240 |
| Scanning(direct,subfields)             |     4/   12/   84 |    3840 |
| Scanning(single_sw_spi)                |   156/  159/  180 |     240 |
| Scanning(single_sw_spi,subfields)      |     4/   20/  180 |    3840 |
| Scanning(single_hw_spi)                |    36/   39/   52 |     240 |
| Scanning(single_hw_spi,subfields)      |     4/    8/   56 |    3840 |
| Scanning(dual_sw_spi)                  |   264/  269/  300 |     240 |
| Scanning(dual_sw_spi,subfields)        |     4/   30/  296 |    3840 |
| Scanning(dual_hw_spi)                  |    24/   26/   40 |     240 |
| Scanning(dual_hw_spi,subfields)        |     4/    7/   40 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(direct_fast)                  |    28/   29/   36 |     240 |
| Scanning(direct_fast,subfields)        |     4/    8/   44 |    3840 |
| Scanning(single_sw_spi_fast)           |    28/   31/   40 |     240 |
| Scanning(single_sw_spi_fast,subfields) |     4/    8/   40 |    3840 |
| Scanning(dual_sw_spi_fast)             |    20/   24/   32 |     240 |
| Scanning(dual_sw_spi_fast,subfields)   |     4/    7/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 22308/22326/22596 |      20 |
| Tm1637(Fast)                           | 21056/21069/21224 |      20 |
+----------------------------------------+-------------------+---------+
```

What is amazing is that if you use `digitalWriteFast()`, the software SPI is
just as fast as hardware SPI, **and** consumes 500 bytes of less flash memory
space.

Here are the CPU numbers for an ESP8266:

```
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |    12/   12/   48 |     240 |
| Scanning(direct,subfields)             |     0/    2/   24 |    3840 |
| Scanning(single_sw_spi)                |    29/   29/   33 |     240 |
| Scanning(single_sw_spi,subfields)      |     0/    4/   43 |    3840 |
| Scanning(single_hw_spi)                |    11/   11/   23 |     240 |
| Scanning(single_hw_spi,subfields)      |     0/    2/   24 |    3840 |
| Scanning(dual_sw_spi)                  |    50/   50/   58 |     240 |
| Scanning(dual_sw_spi,subfields)        |     1/    7/   67 |    3840 |
| Scanning(dual_hw_spi)                  |    12/   12/   24 |     240 |
| Scanning(dual_hw_spi,subfields)        |     1/    2/   28 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 21494/21500/21539 |      20 |
+----------------------------------------+-------------------+---------+
```

On the ESP8266, the hardware SPI is about 4X after, but it does consume 1200
bytes for flash space. But on the ESP8266 flash memory is usually not a concern,
so it seems to make sense to use hardware SPI on the ESP8266.

If we want to drive a 4 digit LED display at 60 frames per second, using a
subfield modulation of 16 subfields per field, we get a field rate of 3.84 kHz,
or 260 microseconds per field. We see from
[examples/AutoBenchmark](examples/AutoBenchmark) that all hardware platforms are
capable of providing a rendering time between fields of less than 260
microseconds.

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

The library is extensively tested on the following boards:

* Arduino Nano clone (16 MHz ATmega328P)
* SparkFun Pro Micro clone (16 MHz ATmega32U4)
* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (72 MHz ARM Cortex-M4)

I will occasionally test on the following hardware as a sanity check:

* Teensy LC (48 MHz ARM Cortex-M0+)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)

<a name="ToolChain"></a>
### Tool Chain

* [Arduino IDE 1.8.13](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.14.0](https://arduino.github.io/arduino-cli)
* [Arduino AVR Boards 1.8.3](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.1](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 1.9.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 2.7.4](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 1.0.4](https://github.com/espressif/arduino-esp32)
* [Teensydino 1.53](https://www.pjrc.com/teensy/td_download.html)

<a name="OperatingSystem"></a>
### Operating System

I use Ubuntu 18.04 and 20.04 for the vast majority of my development. I expect
that the library will work fine under MacOS and Windows, but I have not tested
them.

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="FeedbackAndSupport"></a>
## Feedback and Support

If you have any questions, comments, bug reports, or feature requests, please
file a GitHub ticket instead of emailing me unless the content is sensitive.
(The problem with email is that I cannot reference the email conversation when
other people ask similar questions later.) I'd love to hear about how this
software and its documentation can be improved. I can't promise that I will
incorporate everything, but I will give your ideas serious consideration.

<a name="Authors"></a>
## Authors

Created by Brian T. Park (brian@xparks.net).
