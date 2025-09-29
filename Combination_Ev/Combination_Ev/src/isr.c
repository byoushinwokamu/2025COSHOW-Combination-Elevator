#include "ic165.h"
#include "pinmacro.h"
#include "uart.h"

extern volatile uint16_t swinput;
extern volatile uint16_t door_holding;
extern volatile uint8_t ev_state;
extern volatile uint8_t ev_current_dir;
extern volatile uint8_t ev_current_floor;
extern volatile uint8_t task_queue[5];
volatile uint8_t score_a = 0;
volatile uint8_t score_b = 0;
volatile uint8_t dest_floor = 0;
volatile uint8_t dest_dir = DIR_IDLE;
volatile uint8_t rxbuf = 0;
volatile uint8_t is_req = 0; // 1: Compare to the other E/V, 0: I go

static uint8_t evaluate_score(uint8_t floor, uint8_t dir);

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
    ev_state = ST_DOOR_OPENING;
    door_holding = 0;
  }
  if (swinput & (1 << SW_CAR_CLOSE_BIT))
  {
    if (ev_state != door_holding) return;
    ev_state = ST_DOOR_CLOSING;
    door_holding = 0;
    return;
  }
  if (swinput & (1 << SW_CAR_1F_BIT))
  {
    if (ev_current_dir == DIR_ASCENDING) return;
    enqueue(1, ev_current_dir);
    dest_floor = 1;
    dest_dir = ev_current_dir;
    is_req = 0;
  }
  if (swinput & (1 << SW_CAR_2F_BIT))
  {
    if (ev_current_dir == DIR_ASCENDING && ev_current_floor >= 2) return;
    if (ev_current_dir == DIR_DESCENDING && ev_current_floor <= 2) return;
    enqueue(2, ev_current_dir);
    dest_floor = 2;
    dest_dir = ev_current_dir;
    is_req = 0;
  }
  if (swinput & (1 << SW_CAR_3F_BIT))
  {
    if (ev_current_dir == DIR_ASCENDING && ev_current_floor >= 3) return;
    if (ev_current_dir == DIR_DESCENDING && ev_current_floor <= 3) return;
    dest_floor = 3;
    dest_dir = ev_current_dir;
    is_req = 0;
  }
  if (swinput & (1 << SW_CAR_4F_BIT))
  {
    if (ev_current_dir == DIR_DESCENDING) return;
    dest_floor = 4;
    dest_dir = ev_current_dir;
    is_req = 0;
  }
  if (swinput & (1 << SW_CAR_BELL_BIT))
  {
    // Bell behavior
  }

  if (swinput & (1 << SW_CALL_1F_UP_BIT))
  {
    uart_tx_data(0, 1, DIR_ASCENDING, 0);
    dest_floor = 1;
    dest_dir = DIR_ASCENDING;
    is_req = 1;
  }
  if (swinput & (1 << SW_CALL_2F_UP_BIT))
  {
    uart_tx_data(0, 2, DIR_ASCENDING, 0);
    dest_floor = 2;
    dest_dir = DIR_ASCENDING;
    is_req = 1;
  }
  if (swinput & (1 << SW_CALL_3F_UP_BIT))
  {
    uart_tx_data(0, 3, DIR_ASCENDING, 0);
    dest_floor = 3;
    dest_dir = DIR_ASCENDING;
    is_req = 1;
  }
  if (swinput & (1 << SW_CALL_2F_DOWN_BIT))
  {
    uart_tx_data(0, 2, DIR_DESCENDING, 0);
    dest_floor = 2;
    dest_dir = DIR_DESCENDING;
    is_req = 1;
  }
  if (swinput & (1 << SW_CALL_3F_DOWN_BIT))
  {
    uart_tx_data(0, 3, DIR_DESCENDING, 0);
    dest_floor = 3;
    dest_dir = DIR_DESCENDING;
    is_req = 1;
  }
  if (swinput & (1 << SW_CALL_4F_DOWN_BIT))
  {
    uart_tx_data(0, 4, DIR_DESCENDING, 0);
    dest_floor = 4;
    dest_dir = DIR_DESCENDING;
    is_req = 1;
  }

  if (is_req) // External Button Pushed
  {
    // Evaluate score of current E/V
    score_a = evaluate_score(dest_floor, dest_dir);
    uart_tx_data(score_a, ev_current_floor, ev_current_dir, 0);
  }
  else // Internal Button Pushed
  {
    enqueue(dest_floor, dest_dir);
  }
}

// UART Receive
ISR(USART_RX_vect)
{
  rxbuf = UDR0; // Read data

  if (rxbuf & (1 << UART_SENDER_BIT)) // EVB
  {
    score_a = (rxbuf & (0b11100000U)) >> UART_SCORE_BIT;
    score_b = evaluate_score(rxbuf & 0b11, (rxbuf & 0b100) >> 2);
    if (score_a > score_b) enqueue(rxbuf & 0b11, (rxbuf & 0b100) >> 2);
  }
  else // EVA
  {
    if (rxbuf & (1 << UART_ASSIGN)) // returned
      enqueue(rxbuf & 0b11, (rxbuf & 0b100) >> 2);
  }
}

static uint8_t evaluate_score(uint8_t floor, uint8_t dir)
{
  uint8_t score = 0;
  for (uint8_t i = 0; i < 4; i++)
  {
    if (task_queue[i])
    {
      score += 1;
    }
  }
  return score;
}