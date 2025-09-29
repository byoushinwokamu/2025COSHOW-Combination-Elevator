#include "hx711.h"
#include "ic165.h"
#include "ic595.h"
#include "pinmacro.h"
#include "servo.h"
#include "stepper.h"
#include "uart.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define DOOR_HOLD_TIME 1000

volatile uint16_t swinput = 0xFFFF;
volatile uint16_t door_holding = 0;
volatile uint8_t ev_current_dir = DIR_IDLE;
volatile uint8_t ev_current_floor = 1;
volatile uint8_t ev_state = ST_IDLE;
volatile uint16_t g_light_timer_count = 0; // 3초 조명 타이머를 위한 카운트 변수
volatile uint8_t task_queue[5];

void init();

int main(void)
{
  init();
  while (1)
  {
    switch (ev_state)
    {
    case ST_IDLE:
      break;

    case ST_MOVING:
      break;

    case ST_DOOR_OPENING:
      servo_door_open();
      break;

    case ST_DOOR_OPENED:
      // Hold until time elapsed or close button is pushed
      if (++door_holding == DOOR_HOLD_TIME) ev_state = ST_DOOR_CLOSING;
      break;

    case ST_DOOR_CLOSING:
      servo_door_close();
      break;
    }

    _delay_ms(50);
  }
}

void init()
{
  // 시프트 레지스터 제어핀 설정
  RCLK_595_DDR |= (1 << RCLK_595_PIN);
  SER_595_DDR |= (1 << SER_595_PIN);
  SRCLK_DDR |= (1 << SRCLK_PIN);
  CP_LATCH_165_DDR |= (1 << CP_LATCH_165_PIN);
  MISO_165_DDR &= ~(1 << MISO_165_PIN); // MISO는 입력

  // 리미트 스위치 핀 입력으로 설정
  // 외부 풀업 저항이 있으므로, 내부 풀업(PORTC) 설정은 하지 않음
  LS_HOME_DDR &= ~(1 << LS_HOME_PIN);
  LS_DOOR_CLOSED_DDR &= ~(1 << LS_DOOR_CLOSED_PIN);

  // 스텝모터 제어핀 출력 설정
  STEPPER_1_DDR |= (1 << STEPPER_1_PIN);
  STEPPER_2_DDR |= (1 << STEPPER_2_PIN);
  STEPPER_3_DDR |= (1 << STEPPER_3_PIN);
  STEPPER_4_DDR |= (1 << STEPPER_4_PIN);

  // 서보모터 PWM 핀 출력 설정
  SERVO_DDR |= (1 << SERVO_PIN);

  // HX711 로드셀 핀 설정
  HX711_DT_DDR &= ~(1 << HX711_DT_PIN);  // DT 입력
  HX711_SCK_DDR |= (1 << HX711_SCK_PIN); // SCK 출력

  // UART 핀 설정
  UART_TX_DDR |= (1 << UART_TX_PIN);  // TX 출력
  UART_RX_DDR &= ~(1 << UART_RX_PIN); // RX 입력

  // 인터럽트 초기화 필요?

  // 초기 출력 상태 업데이트
  ic595_update();
  _delay_ms(100);
}