#include "ic595.h"

volatile static uint32_t output_buf = 0xffffffff;

void ic595_update()
{
  RCLK_595_PORT &= ~(1 << RCLK_595_PIN); // Latch LOW

  for (uint8_t i = 0; i < 32; i++)
  {
    if (output_buf & (1UL << (31 - i)))
    {
      SER_595_PORT |= (1 << SER_595_PIN);
    }
    else
    {
      SER_595_PORT &= ~(1 << SER_595_PIN);
    }
    SRCLK_PORT |= (1 << SRCLK_PIN);
    SRCLK_PORT &= ~(1 << SRCLK_PIN);
  }
  RCLK_595_PORT |= (1 << RCLK_595_PIN); // Latch HIGH
}

void ic595_fndset(uint8_t num)
{
  if (num > 9) // Out of bound
  {
    output_buf |= (0b1111UL << SEG_A_BIT);
    return;
  }
  else // Set bits after clear
  {
    output_buf &= ~(0b1111UL << SEG_A_BIT);
    output_buf |= ((uint32_t)num << SEG_A_BIT);
  }
}

uint8_t ic595_fndget()
{
  // Return current FND number
  // Is this function needed??
}

void ic595_ledset(uint8_t pos, uint8_t state)
{
  // Active-Low structure
  if (state)
    output_buf &= ~(1UL << pos);
  else
    output_buf |= (1UL << pos);
}