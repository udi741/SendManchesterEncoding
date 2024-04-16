# Manchester Encoder Module

## Overview
This module allows you to transmit Manchester encoded data using an nRF52832 Feather microcontroller.

## Usage
1. Include the `senderWrapper.h` header file in your Arduino sketch.
2. Set up the output pin for transmission with `setOutputPin()`.
3. Configure the timer interrupt with `setupTimerIRQ()`.
4. Call `transmitManchesterData()` to send encoded data.
5. Call `transmitManchesterDataError()` to simulate transmission errors.
6. Call `transmitManchesterDataSizeError()` to simulate size errors.

## Example
```c
#include "senderWrapper.h"

#define OUTPUT_PIN 8

void setup() {
  setOutputPin(OUTPUT_PIN);
  setupTimerIRQ(); 
}

void loop() {
  // Transmit Manchester encoded data
  transmitManchesterData();

  // Simulate transmission with an error
  transmitManchesterDataError();

  // Simulate transmission with a size error
  transmitManchesterDataSizeError();
}
```
