<<<<<<< HEAD
#include "ic165.h"
#include "ic595.h"
#include "pinmacro.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

=======
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

>>>>>>> main
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
<<<<<<< HEAD
    // 모든 스위치 상태 읽기
    swinput = ic165_read();
    // --------------------------------------------------------------------------
    // --- 버튼 입력 처리 로직 ---
    // --------------------------------------------------------------------------
    // 카 내부 1층 버튼
    if (!(swinput & (1 << SW_CAR_1F_BIT)))
    {
      ic595_ledset(LED_CAR_1F_BIT, 1);
      ic595_fndset(1);
    }
    // 카 내부 2층 버튼
    else if (!(swinput & (1 << SW_CAR_2F_BIT)))
    {
      ic595_ledset(LED_CAR_2F_BIT, 1);
      ic595_fndset(2);
    }
    // 카 내부 3층 버튼
    else if (!(swinput & (1 << SW_CAR_3F_BIT)))
    {
      ic595_ledset(LED_CAR_3F_BIT, 1);
      ic595_fndset(3);
    }
    // 카 내부 4층 버튼
    else if (!(swinput & (1 << SW_CAR_4F_BIT)))
    {
      ic595_ledset(LED_CAR_4F_BIT, 1);
      ic595_fndset(4);
    }
    // 외부 1층 호출 (UP)
    else if (!(swinput & (1 << SW_CALL_1F_UP_BIT)))
    {
      ic595_ledset(LED_CALL_1F_UP_BIT, 1);
    }
    // 외부 2층 호출 (UP)
    else if (!(swinput & (1 << SW_CALL_2F_UP_BIT)))
    {
      ic595_ledset(LED_CALL_2F_UP_BIT, 1);
    }
    // 외부 2층 호출 (DOWN)
    else if (!(swinput & (1 << SW_CALL_2F_DOWN_BIT)))
    {
      ic595_ledset(LED_CALL_2F_DOWN_BIT, 1);
    }
    // 외부 3층 호출 (UP)
    else if (!(swinput & (1 << SW_CALL_3F_UP_BIT)))
    {
      ic595_ledset(LED_CALL_3F_UP_BIT, 1);
    }
    // 외부 3층 호출 (DOWN)
    else if (!(swinput & (1 << SW_CALL_3F_DOWN_BIT)))
    {
      ic595_ledset(LED_CALL_3F_DOWN_BIT, 1);
    }
    // 외부 4층 호출 (DOWN)
    else if (!(swinput & (1 << SW_CALL_4F_DOWN_BIT)))
    {
      ic595_ledset(LED_CALL_4F_DOWN_BIT, 1);
    }
    // 카 내부 문 열림 버튼
    else if (!(swinput & (1 << SW_CAR_OPEN_BIT)))
    {
      ic595_ledset(LED_CAR_OPEN_BIT, 1);
    }
    // 카 내부 문 닫힘 버튼
    else if (!(swinput & (1 << SW_CAR_CLOSE_BIT)))
    {
      ic595_ledset(LED_CAR_CLOSE_BIT, 1);
    }
    // 카 내부 비상벨 버튼
    else if (!(swinput & (1 << SW_CAR_BELL_BIT)))
    {
      ic595_ledset(LED_CAR_BELL_BIT, 1);
    }
    // 아무 버튼도 눌리지 않았을 경우
    else
    {
      // 모든 층/호출/기능 LED를 끔
      ic595_ledset(LED_CAR_1F_BIT, 0);
      ic595_ledset(LED_CAR_2F_BIT, 0);
      ic595_ledset(LED_CAR_3F_BIT, 0);
      ic595_ledset(LED_CAR_4F_BIT, 0);
      ic595_ledset(LED_CALL_1F_UP_BIT, 0);
      ic595_ledset(LED_CALL_2F_UP_BIT, 0);
      ic595_ledset(LED_CALL_2F_DOWN_BIT, 0);
      ic595_ledset(LED_CALL_3F_UP_BIT, 0);
      ic595_ledset(LED_CALL_3F_DOWN_BIT, 0);
      ic595_ledset(LED_CALL_4F_DOWN_BIT, 0);
      ic595_ledset(LED_CAR_OPEN_BIT, 0);
      ic595_ledset(LED_CAR_CLOSE_BIT, 0);
      ic595_ledset(LED_CAR_BELL_BIT, 0);
      // (참고: 실제 엘리베이터는 현재 층을 표시해야 하므로,
      //  이 부분은 나중에 '현재 층' 변수를 받아 처리하도록 수정해야 합니다.)
      ic595_fndset(10); // 7세그먼트 끄기
=======
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
>>>>>>> main
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
  LS_DOOR_OPEN_DDR &= ~(1 << LS_DOOR_OPEN_PIN);
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

  // IR 송수신기 핀 설정
  IR_TX_DDR |= (1 << IR_TX_PIN);  // IR TX 출력
  IR_RX_DDR &= ~(1 << IR_RX_PIN); // IR RX 입력

  // UART 핀 설정
  UART_TX_DDR |= (1 << UART_TX_PIN);  // TX 출력
  UART_RX_DDR &= ~(1 << UART_RX_PIN); // RX 입력

  // 인터럽트 초기화 필요?

  // 초기 출력 상태 업데이트
  ic595_update();
  _delay_ms(100);
}