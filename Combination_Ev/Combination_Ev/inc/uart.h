#ifndef _UART_H_
#define _UART_H_

#include "pinmacro.h"

#include <avr/io.h>
#include <stdint.h>

#define ASSIGN_OK 0b00000000U
#define ASSIGN_NO 0b11111111U

void uart_init(uint16_t baudrate);
void uart_tx_byte(uint8_t data);
uint8_t uart_rx_byte();
void uart_tx_status(uint8_t stat);
void uart_tx_assign(uint8_t asgn);

#endif