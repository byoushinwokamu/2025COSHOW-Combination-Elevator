/*
 * UART driver for ATmega328P (F_CPU = 16 MHz)
 * - Mode: 8N1, Double Speed (U2X0 = 1)
 * - Blocking TX/RX
 * - '\n' 전송 시 CRLF로 변환 (터미널 호환)
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include "uart.h"
#include "pinmacro.h"   // PD0: RX, PD1: TX

// U2X0=1 기준 보오율 레지스터 계산 (반올림)
static inline uint16_t uart_calc_ubrr(uint32_t baud)
{
    // UBRR = round(F_CPU / (8*baud) - 1)
    uint32_t ubrr = (F_CPU + (baud * 4)) / (8UL * baud) - 1UL;
    if (ubrr > 4095UL) ubrr = 4095UL; // 12비트 가드
    return (uint16_t)ubrr;
}

void uart_init(uint16_t baudrate)
{
    // 핀 방향 설정 (TX=출력, RX=입력)
    UART_TX_DDR |= (1 << UART_TX_PIN);
    UART_RX_DDR &= ~(1 << UART_RX_PIN);

    // 더블 스피드 활성화(U2X0=1)
    UCSR0A = (1 << U2X0);

    // 보오율 설정
    uint16_t ubrr = uart_calc_ubrr(baudrate);
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);

    // 프레임: 8비트 데이터, 패리티 없음, 스톱 1비트 (8N1)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    // 송수신 활성화
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

void uart_transmit(uint8_t data)
{
    // 송신 버퍼가 빌 때까지 대기
    while (!(UCSR0A & (1 << UDRE0))) {;}
    UDR0 = data;
}

uint8_t uart_receive(void)
{
    // 수신 데이터가 올 때까지 대기
    while (!(UCSR0A & (1 << RXC0))) {;}
    return UDR0;
}

void uart_send_string(const char *str)
{
    while (*str)
    {
        // 개행 변환: '\n' -> "\r\n" (터미널 호환성 향상)
        if (*str == '\n') uart_transmit('\r');
        uart_transmit((uint8_t)*str++);
    }
}
