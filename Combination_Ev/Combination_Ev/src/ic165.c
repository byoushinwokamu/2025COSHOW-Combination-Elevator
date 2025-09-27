#include "ic165.h"

uint16_t ic165_read()
{
  uint16_t data = 0;

  CP_LATCH_165_PORT &= ~(1 << CP_LATCH_165_PIN); // Load LOW
  _delay_us(5);
  CP_LATCH_165_PORT |= (1 << CP_LATCH_165_PIN); // Load HIGH (데이터 캡처)

  for (uint8_t i = 0; i < 16; i++)
  {
    data <<= 1;
    if (MISO_165_PIN_REG & (1 << MISO_165_PIN))
    {
      data |= 1;
    }
    SRCLK_PORT |= (1 << SRCLK_PIN);
    SRCLK_PORT &= ~(1 << SRCLK_PIN);
  }
  return data;
}