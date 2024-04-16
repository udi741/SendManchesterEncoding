#include "senderWrapper.h"

static uint8_t byteIdx = 0;
static uint8_t bitIdx = 8;
static bool transmitting = false;
static byte encodedData[MAX_BYTES_TO_SEND*2];
static uint8_t endByteIdx = 0;
static bool stopCondition = false;
static bool sendStartBits = false;
static int startBitsCnt = 0;
static uint8_t outputPin;

/**
 * @brief Set the output pin for transmitting Manchester encoded data.
 *
 * This function sets the output pin for transmitting Manchester encoded data.
 *
 * @param _outputPin The output pin number.
 */
void setOutputPin(uint8_t _outputPin)
{
  outputPin = _outputPin;
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);
}

/**
 * @brief Setup timer register.
 *
 * This function sets up the timer register to generate interrupt every GAP_TIME_US in order to transmit the data. 
 * GAP_TIME_US = 1/baud
 */
void setupTimerIRQ()
{
	NVIC_EnableIRQ(TIMER0_IRQn);
	NRF_TIMER0->TASKS_STOP = 1;
	
	NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer; // use timer mode - increment timer counter every tick
	NRF_TIMER0->BITMODE = (TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos);
	NRF_TIMER0->PRESCALER = 4; // 16MHz / (2^4) = 1MHz -> 1us
	
	NRF_TIMER0->TASKS_CLEAR = 1;
	NRF_TIMER0->CC[0] = GAP_TIME_US;
	NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos; // enable compare[0] event

	NRF_TIMER0->SHORTS = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos); // clears timer on comapre[0] event
}

/**
 * @brief Transmit start bits.
 *
 * This function transmits start bits for synchronization. 
 * The bits that transmitted are 0->0->0->0->1->0->1
 */
void startBitsTransmission()
{
  if (startBitsCnt < START_ZERO_AMOUNT || startBitsCnt == START_ZERO_AMOUNT+1)
  {
    digitalWrite(outputPin, 0);
  }
  else if (startBitsCnt == START_ZERO_AMOUNT || startBitsCnt == START_ZERO_AMOUNT+2)
  {
    digitalWrite(outputPin, 1);
  }
  if (++startBitsCnt == START_ZERO_AMOUNT+3)
  {
    sendStartBits = false;
    startBitsCnt = 0;
  }
}
/**
 * @brief Transmit stop bits.
 *
 * This function transmits stop bits after data transmission.
 * The bits that transmitted are 0->0->0->0
 */
void stopBitsTransmission()
{
  if (startBitsCnt < START_ZERO_AMOUNT)
  {
    digitalWrite(outputPin, 0);
  }
  startBitsCnt++;
  if (startBitsCnt == START_ZERO_AMOUNT)
  {
    transmitting = false;
    stopCondition = false;
    sendStartBits = true;
    startBitsCnt = 0;
  }
}

/**
 * @brief Transmit bits from byte.
 *
 * This function transmits bits from the data(byte) one by one.
 */
void transmitBit()
{
  uint8_t state = (encodedData[byteIdx] >> --bitIdx) & 0x01;
  digitalWrite(outputPin, state);
  if (byteIdx > endByteIdx)
  {
    stopCondition = true;
  }
  if (bitIdx == 0)
  {
    bitIdx = 8;
    byteIdx++;
  }
}

/**
 * @brief Transmit Manchester encoded data.
 *
 * This function transmits Manchester encoded data.
 */
void transmitManchesterData()
{
  if (!transmitting)
  {
    NRF_TIMER0->TASKS_STOP = 1;
    NRF_TIMER0->TASKS_CLEAR = 1;
    byteIdx = 0;
    bitIdx = BYTE_SIZE;
    byte data[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x90};
    

    enum StatusMessages status;
    status = encodeManchester(data, ARRAY_LENGTH(data), encodedData, STANDARD);
    if (status == OK)
    {
      startBitsTransmission();
      endByteIdx = ARRAY_LENGTH(encodedData)-1;
      transmitting = true;
      NRF_TIMER0->TASKS_START = 1;
    }
  }
}

/**
 * @brief Transmit Manchester encoded data with error.
 *
 * This function transmits Manchester encoded data with an INVALID_ENCODE error for testing purposes.
 */
void transmitManchesterDataError()
{
  if (!transmitting)
  {
    NRF_TIMER0->TASKS_STOP = 1;
    NRF_TIMER0->TASKS_CLEAR = 1;
    byteIdx = 0;
    bitIdx = BYTE_SIZE;
    encodedData[0] = 0x59;
    encodedData[1] = 0xFF;
    startBitsTransmission();
    endByteIdx = 1;
    transmitting = true;
    NRF_TIMER0->TASKS_START = 1;
  }
}

/**
 * @brief Transmit Manchester encoded data with size error.
 *
 * This function transmits Manchester encoded data with a size of the data error for testing purposes.
 */
void transmitManchesterDataSizeError()
{
  if (!transmitting)
  {
    NRF_TIMER0->TASKS_STOP = 1;
    NRF_TIMER0->TASKS_CLEAR = 1;
    byteIdx = 0;
    bitIdx = BYTE_SIZE;
    encodedData[0] = 0x59;
    encodedData[1] = 0x56;
    encodedData[2] = 0x59;
    startBitsTransmission();
    endByteIdx = 2;
    transmitting = true;
    NRF_TIMER0->TASKS_START = 1;
  }
}

extern "C"
{
  /**
  * @brief Timer interrupt handler.
  */
  void TIMER0_IRQHandler() {
    // Check if compare event has occurred
    if (NRF_TIMER0->EVENTS_COMPARE[0] && NRF_TIMER0->INTENSET & TIMER_INTENSET_COMPARE0_Msk) {
      NRF_TIMER0->EVENTS_COMPARE[0] = 0;
      if (sendStartBits)
      {
        startBitsTransmission();
      }    
      else if (stopCondition)
      {
        stopBitsTransmission();
      }
      else
      {
        transmitBit();
      }
    }
  }
}