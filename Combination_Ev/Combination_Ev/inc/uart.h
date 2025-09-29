#ifndef _UART_H_
#define _UART_H_

#include "pinmacro.h"

#include <avr/io.h>
#include <stdint.h>

void uart_init(uint16_t baudrate);
void uart_tx_byte(uint8_t data);
uint8_t uart_rx_byte();
void uart_tx_data(uint8_t score, uint8_t floor, uint8_t dir, uint8_t assign);
void enqueue(uint8_t floor, uint8_t dir);
void dequeue();

#endif