#!/usr/bin/python3
#
# Python script that regenerates the README.md from the embedded template. Uses
# ./generate_table.awk to regenerate the ASCII tables from the various *.txt
# files.

from subprocess import check_output

nano_results = check_output(
    "./generate_table.awk < nano.txt", shell=True, text=True)
micro_results = check_output(
    "./generate_table.awk < micro.txt", shell=True, text=True)
samd_results = check_output(
    "./generate_table.awk < samd.txt", shell=True, text=True)
stm32_results = check_output(
    "./generate_table.awk < stm32.txt", shell=True, text=True)
esp8266_results = check_output(
    "./generate_table.awk < esp8266.txt", shell=True, text=True)
esp32_results = check_output(
    "./generate_table.awk < esp32.txt", shell=True, text=True)
teensy32_results = check_output(
    "./generate_table.awk < teensy32.txt", shell=True, text=True)
#teensy32_results = 'TBD'

print(f"""\
# AutoBenchmark

This program creates instances of various subclasses `LedModule` using different
configurations:

* `DirectModule`: group and segment pins directly connected to MCU
* `DirectFast4Module`: same as `DirectModule` but using `digitalWriteFast` library
* `HybridModule`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through SPI
* `Hc595Module`: group pins and segment pins connected to two 74HC595
  which are accessed through SPI
* `Tm1637Module`: an LED module using a TM1637 driver chip, accessed through
  a two-wire protocol similar to I2C
* `Max7219Module`: an LED module using a MAX7219 driver chip, accessed through
  SPI

We then measure the time taken by the methods used to render the digit to the
LED module:

* Subclasses of `ScanningModule` (`Hc595Module`, `HybridModule`, `DirectModule`)
    * Measure the time taken by `ScanningModule::renderFieldNow()` which renders
      a single digit (multiple fields make up a frame (a single frame is the
      complete rendering of all digits on the display module).
* `Tm1637Module::flush()
    * Sends out the buffered digits over the custom two-wire protocol.
* `Max7219Module::flush()`
    * Sends out the buffered digits using SPI.

**Version**: AceSegment v0.6

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

* [AceCommon](https://github.com/bxparks/AceCommon)
* [AceSegment](https://github.com/bxparks/AceButton)

On AVR processors, the following library is required to run the
`digitalWriteFast()` versions of the low-level drivers:

* [digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast)

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules to generate the `*.txt` results file for several
microcontrollers that I usually support, but the `$ make benchmarks` command
does not work very well because the USB port of the microcontroller is a
dynamically changing parameter. I created a semi-automated way of collect the
`*.txt` files:

1. Connect the microcontroller to the serial port. I usually do this through a
USB hub with individually controlled switch.
2. Type `$ auniter ports` to determine its `/dev/ttyXXX` port number (e.g.
`/dev/ttyUSB0` or `/dev/ttyACM0`).
3. If the port is `USB0` or `ACM0`, type `$ make nano.txt`, etc.
4. Switch off the old microontroller.
5. Go to Step 1 and repeat for each microcontroller.

The `generate_table.awk` program reads one of `*.txt` files and prints out an
ASCII table that can be directly embedded into this README.md file. For example
the following command produces the table in the Nano section below:

```
$ ./generate_table.awk < nano.txt
```

Fortunately, we no longer need to run `generate_table.awk` for each `*.txt`
file. The process has been automated using the `generate_readme.py` script which
will be invoked by the following command:
```
$ make README.md
```

The CPU times below are given in microseconds. The "samples" column is the
number of `TimingStats::update()` calls that were made.

## CPU Time Changes

**v0.4:**
* Huge refactoring of core AceSegment classes. Rewrote AutoBenchmark
  to match similar programs in the AceButton, AceCrc and AceTime libraries.

**v0.5:**

* Add benchmarks for `Tm1637Module`.
    * The CPU time is mostly determined by the calls to `delayMicroseconds()`,
      which is must be about 100 microseconds due to the unusually large
      capacitors (10 nF) installed on the DIO and CLK lines. They should have
      been about 100X smaller (200 pF).
    * Benchmarks both 4-digit and 6-digit LED modules given separately
      because the `Tm1637::flush()` method is roughtly proportional to the
      number of digits.
    * Benchmarks for `Tm1637::flushIncremental()` is independent of the total
      number of digits because only a single digit is sent for each call.
      The maximum time for a single call to `flushIncremental()` is about 10 ms.
    * For a 6-digit module, `flush()` can take as much as 27-30 milliseconds (!)
      which uncomfortably close to the maximum amount of time before an ESP8266
      resets via the watch dog timer. On ESP8266 (and maybe others), the
      `flushIncremental()` should be used.
* Add benchmarks for `DirectModule`, `DirectFast4Module`, `HybridModule` with 4
  digits, because these are custom made by hand.
* Add `Max7219Module` with 8 digits as found on off-the-shelf LED modules.
  Duration of `flush()` almost doubles from 4 digits to 8 digits obviously.
* Add `Hc595Module` with 8 digits as found on off-the-shelf LED modules.
  Duration of `renderFieldWhenReady()` does not change from 4 digits to 8 digits
  because it renders a single digit at time.
* Upgrade from ESP32 Core v1.0.4 to v1.0.6.
* Adding `byteOrder` and `remapArray` parameters to `Hc595Module` increases
  the CPU time of `renderFieldsNow()` by a tiny amount, maybe a microsecond on a
  AVR. For 32-bit processors, the difference seems to be within the noise.

**v0.6:**

* `HardSpiInterface` is slightly slower on the fastest processors (e.g. ESP8266,
  ESP32, Teensy 3.2), because the SPI frequency was reduced from 20 MHz to 8
  MHz. No difference on the slower processors. Those fast processors can
  actually sustain a 20 MHz SPI, which breaks the MAX7219 chip because it can
  handle only 16 MHz.
* Verified that removing the 10 nF capacitors from the CLK and DIO lines of the
  TM1637 modules allows a much shorter `BIT_DELAY`. Added CPU benchmark numbers
  for `BIT_DELAY = 5` microseconds (e.g. `Tm1637(4,SoftTmi,5us)` and
  `Tm1637(4,SoftTmiFast,5us)`). The `flush()` or `flushIncremental()` durations
  are almost a factor of 10X to 20X shorter compared to `BIT_DELAY = 100`.

**v0.6+:**

* Add `sizeof(LevelWriter)` and `sizeof(HardWireInterface)`.
* Add benchmarks for `Ht16k33Module`. On SAMD21 and STM32, the runtime
  of the `Ht16k33Module` seems to depend on whether an actual HT16K33 LED module
  is attached to the I2C bus. On the SAMD21, the transmission time becomes 50X
  longer *without* the LED module attached. On the STM32, gthe transmission time
  becomes 30-40X smaller *without* the LED module attached.

## Results

The following tables show the number of microseconds taken by:

* `DirectModule::renderFieldNow()`, `HybridModule::renderFieldNow()`,
  `Hc595Module::renderFieldNow()`
    * renders the 8 segments of a single LED digit.
    * If the LED module has 4 digits, then `renderFieldNow()` must be called 4
      times to render the light pattern of the entire LED module. The entire
      rendering is then called a frame.
    * Most people can no longer see flickering of the display at about 60 frames
      a second. To achieve that, the `renderFieldNow()` method must be called
      240 times a second for a module with 4 digits, or every 4.17 milliseconds.
    * The results below show that every processor, even the slowest AVR
      processor, is able to meet this threshhold.
* `Tm1637Module::flush()` or `Tm1637Module::flushIncremental()`
    * sends digits in the buffer to the TM1637 LED module using the I2C-like
      protocol
    * results for two values of `BIT_DELAY` are collected
        * 100 microseconds (e.g. `Tm1637(4,SoftTmi)`)
        * 5 microseconds (e.g. `Tm1637(4,SoftTmi,5us)`)
* `Max7219Module::flush()`
    * sends all digits in the buffer to the MAX7219 LED module using
      software SPI or hardware SPI

On AVR processors, the "fast" options are available using the
[digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast) library whose
`digitalWriteFast()` functions can be up to 50X faster if the `pin` number and
`value` parameters are compile-time constants. In addition, the
`digitalWriteFast` functions reduce flash memory consumption by 600-700 bytes
for `SoftTmiFastInterface`, `SoftSpiFastInterface`, and `HardSpiFastInterface`
compared to their non-fast equivalents.

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3
* `micros()` has a resolution of 4 microseconds

```
{nano_results}
```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
{micro_results}
```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
{samd_results}
```

### STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
{stm32_results}
```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
{esp8266_results}
```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.6

```
{esp32_results}
```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
{teensy32_results}
```
""")
