#include "senderWrapper.h"

#define OUTPUT_PIN 8

void setup()
{
  setOutputPin(OUTPUT_PIN);
  setupTimerIRQ(); 
}

void loop(){
  // Transmit Manchester encoded data
  transmitManchesterData();

  // Simulate transmission with an error
  transmitManchesterDataError();

  // Simulate transmission with a size error
  transmitManchesterDataSizeError();
}