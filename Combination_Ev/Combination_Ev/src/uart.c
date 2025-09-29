#include "uart.h"

extern volatile uint8_t task_queue[5];
extern volatile uint8_t ev_current_dir;

void uart_init(uint16_t baudrate)
{
  // 1x mode
  UCSR0A &= ~(1 << U2X0);

  // Baud rate 31250bps
  UBRR0H = 0x00;
  UBRR0L = 31;

  // 8bit
  UCSR0C |= 0x06;

  // transmit enable
  UCSR0B |= (1 << TXEN0);

  // receive(interrupt) enable
  UCSR0B |= (1 << RXEN0) | (1 << RXCIE0);
}

void uart_tx_byte(uint8_t data)
{
  while (!(UCSR0A & (1 << UDRE0))); // 송신 준비 완료까지 대기
  UDR0 = data;
}

uint8_t uart_rx_byte()
{
  while (!(UCSR0A & (1 << RXC0))); // 수신 완료까지 대기
  return UDR0;
}

void uart_tx_data(uint8_t score, uint8_t floor, uint8_t dir, uint8_t assign)
{
  uart_tx_byte((floor << UART_FLOOR_BIT) | (dir < UART_DIRECTION_BIT) | (assign << UART_ASSIGN_BIT));
}

void enqueue(uint8_t floor, uint8_t dir)
{
  floor <<= UART_FLOOR_BIT;
  dir <<= UART_DIRECTION_BIT;
  for (uint8_t i = 0; i < 4; i++)
  {
    if (task_queue[i])
    {
      if ((dir & (1 << 7)) != (ev_current_dir & (1 << 7))) // Opposite
        continue;
      else if (dir & (1 << 7)) // Descending
      {
        if (floor < (task_queue[i] & (0b11 << 5))) // Under
          continue;
      }
      else // Ascending
      {
        if (floor > (task_queue[i] & (0b11 << 5))) // Over
          continue;
      }
    }
    task_queue[i] = (floor | dir);
    break;
  }
}

void dequeue()
{
  task_queue[0] = task_queue[1];
  task_queue[1] = task_queue[2];
  task_queue[2] = task_queue[3];
  task_queue[3] = task_queue[4];
  task_queue[4] = 0;
}