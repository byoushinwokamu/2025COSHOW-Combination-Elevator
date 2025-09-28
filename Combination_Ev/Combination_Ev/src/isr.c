#include "ic165.h"
#include "pinmacro.h"

extern volatile uint16_t swinput;
volatile uint8_t score = 0;

// PC3: Home Sw.
// PC4: Door Sensor Sw.
ISR(PCINT1_vect)
{
  ;
}

// PD5: Any Switch Int.
ISR(PCINT2_vect)
{
  swinput = ic165_read();

  // Evaluate score of current E/V
  score = (uint8_t)swinput;
}

// UART Receive
ISR(USART_RX_vect)
{
  ;
}