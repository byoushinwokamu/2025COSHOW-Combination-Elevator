#ifndef _UART_H_
#define _UART_H_

#include "pinmacro.h"

#include <stdint.h>

void uart_init(uint16_t baudrate);
void uart_transmit(uint8_t data);
uint8_t uart_receive();
void uart_send_string(const char *str);

#endif