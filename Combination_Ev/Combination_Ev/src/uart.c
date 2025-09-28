#include "uart.h"

#define HEADER_STATUS 0b00000000U
#define HEADER_ASSIGN 0b11111111U

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

  // receive enable
  UCSR0B |= (1 << RXEN0);
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

void uart_tx_status(uint8_t stat)
{
  uart_tx_byte(HEADER_STATUS);
  uart_tx_byte(stat);
}

void uart_tx_assign(uint8_t asgn)
{
  uart_tx_byte(HEADER_ASSIGN);
  uart_tx_byte(asgn);
}