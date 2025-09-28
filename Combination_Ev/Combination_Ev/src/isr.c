#include "ic165.h"
#include "pinmacro.h"
#include "uart.h"

// #define EV_RX 0
// #define EV_TX 1

extern volatile uint16_t swinput;
extern volatile uint16_t door_holding;
extern volatile uint8_t ev_status;
volatile uint8_t score_a = 0;
volatile uint8_t score_b = 0;
// volatile uint8_t evrt = EV_RX; // distinguish between EV A / EV B
volatile uint16_t rxbuf = 0;

// PC3: Home Sw.
// PC4: Door Sensor Sw.
ISR(PCINT1_vect)
{
  ;
}

// PD5: Any Switch Int.
ISR(PCINT2_vect)
{
  // Read switch input
  swinput = ic165_read();

  // Process input
  if (swinput & (1 << SW_CAR_OPEN_BIT))
  {
    ev_status = ST_DOOR_OPENING;
    door_holding = 0;
  }
  if (swinput & (1 << SW_CAR_CLOSE_BIT))
  {
    if (ev_status != door_holding) break;
    ev_status = ST_DOOR_CLOSING;
    door_holding = 0;
  }
  if (swinput & (1 << SW_CAR_1F_BIT))
  {
  }
  if (swinput & (1 << SW_CAR_2F_BIT))
  {
  }
  if (swinput & (1 << SW_CAR_3F_BIT))
  {
  }
  if (swinput & (1 << SW_CAR_4F_BIT))
  {
  }
  if (swinput & (1 << SW_CAR_BELL_BIT))
  {
  }

  if (swinput & (1 << SW_CALL_1F_UP_BIT))
  {
  }
  if (swinput & (1 << SW_CALL_2F_UP_BIT))
  {
  }
  if (swinput & (1 << SW_CALL_3F_UP_BIT))
  {
  }
  if (swinput & (1 << SW_CALL_2F_DOWN_BIT))
  {
  }
  if (swinput & (1 << SW_CALL_3F_DOWN_BIT))
  {
  }
  if (swinput & (1 << SW_CALL_4F_DOWN_BIT))
  {
  }

  // Evaluate score of current E/V
  score = (uint8_t)swinput;
}

// UART Receive
ISR(USART_RX_vect)
{
  UCSR0A; // Error flag
  rxbuf = (rxbuf << 8) | UDR0;
  if (!((rxbuf >> 8) & 0xF0)) // Isn't a header sign?
    return;                   // Then wait for next byte

  if ((rxbuf >> 8) & 0xF0) // HEADER_ASSIGN; This is E/V A
  {
    if (rxbuf & 0x0F) // ASSIGN_NO: I must go
    {
      ///////////// Add to queue
    }
  }
  else // HEADER_STATUS; This is E/V B
  {
    // ******************* NEED TO EVALUATE STAT *********
    // AND COMPARE
    uart_tx_status(0);

    if (1)
    {
      //////////// Add to queue
      uart_tx_assign(ASSIGN_OK); // I will go
    }
    else
      uart_tx_assign(ASSIGN_NO); // You must go
  }
  if ((rxbuf >> 8) & 0xF0) // HEADER_ASSIGN
  {
  }
  else // HEADER_STATUS
  {
    if (rxbuf & 0xFF > 0)        // Compare with my status
      uart_tx_assign(ASSIGN_OK); // I will go
    else
      uart_tx_assign(ASSIGN_NO); // You must go
  }
}