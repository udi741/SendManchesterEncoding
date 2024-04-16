#ifndef SENDERWRAPPER_H
#define SENDERWRAPPER_H

#define GAP_TIME_US 104
#define START_ZERO_AMOUNT 4
#define MAX_BYTES_TO_SEND 10
#define STANDARD IEEE

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <stdint.h>
#include <stdbool.h>
#include "ManchesterEncoding.h"

void startBitsTransmission();
void stopBitsTransmission();
void transmitBit();
void transmitManchesterData();
void transmitManchesterDataError();
void transmitManchesterDataSizeError();
void setupTimerIRQ();
void setOutputPin(uint8_t _outputPin);

#endif