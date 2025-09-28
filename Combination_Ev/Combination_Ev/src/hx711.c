#include "hx711.h"
#include "pinmacro.h"
#include <util/delay.h>

void hx711_init() {
    HX711_SCK_DDR |= (1 << HX711_SCK_PIN);      // SCK 핀 출력
    HX711_DOUT_DDR &= ~(1 << HX711_DOUT_PIN);   // DOUT 핀 입력
    HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN);    // SCK LOW
}

uint32_t hx711_read() {
    uint32_t data = 0;
    // DOUT이 LOW가 될 때까지 대기
    while (HX711_DOUT_PIN_REG & (1 << HX711_DOUT_PIN));
    // 24비트 데이터 읽기
    for (uint8_t i = 0; i < 24; i++) {
        HX711_SCK_PORT |= (1 << HX711_SCK_PIN);      // SCK HIGH
        _delay_us(1);
        data = (data << 1) | ((HX711_DOUT_PIN_REG & (1 << HX711_DOUT_PIN)) ? 1 : 0);
        HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN);     // SCK LOW
        _delay_us(1);
    }
    // 채널/이득 설정용 추가 클럭 (A채널 128)
    HX711_SCK_PORT |= (1 << HX711_SCK_PIN);
    _delay_us(1);
    HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN);
    _delay_us(1);

    // 24비트 부호 확장
    if (data & 0x800000) data |= 0xFF000000;
    return data;
}

int32_t hx711_read_average(uint8_t times) {
    int64_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        sum += hx711_read();
        _delay_ms(1);
    }
    return (int32_t)(sum / times);
}