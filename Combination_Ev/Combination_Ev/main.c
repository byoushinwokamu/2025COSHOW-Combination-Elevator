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

volatile uint32_t state = 0;
void init();

int main(void)
{
  init();
  while (1)
  {
    swinput = ic165_read();

    // 카 내부 1층 버튼
    if (!(swinput & (1 << SW_CAR_1F_BIT)))
    {
      state = 7;
    }
    // 카 내부 4층 버튼
    else if (!(swinput & (1 << SW_CAR_4F_BIT)))
    {
      state = 3;
    }
    // 외부 2층 호출 (UP)
    else if (!(swinput & (1 << SW_CALL_2F_UP_BIT)))
    {
      state = 1;
    }
    // 카 내부 문 닫힘 버튼
    else if (!(swinput & (1 << SW_CAR_CLOSE_BIT)))
    {
      while (loadcell_is_overload())
      {
        ic595_fndset(10);
        ic595_update();
        _delay_ms(500);
        ic595_fndset(2);
        ic595_update();
        _delay_ms(500);
      }
      if (state == 3) state = 4;
    }
    // 카 내부 문 열림 버튼
    else if (!(swinput & (1 << SW_CAR_OPEN_BIT)))
    {
      if (state == 12) state++;
    }
    else if ((swinput & (1 << SW_CAR_OPEN_BIT)))
    {
      if (state == 13) state++;
    }
    // 아무 버튼도 눌리지 않았을 경우
    else
    {
    }

    switch (state)
    {
    case 0:
      ic595_fndset(1);
      break;
    case 1:
      ic595_ledset(LED_CALL_2F_UP_BIT, 1);
      ic595_ledset(LED_LNT_2F_UP_BIT, 1);
      ic595_update();
      stepper_move_to_floor(2, 1);
      ic595_fndset(2);
      ic595_update();
      state = 2;
      break;
    case 2:
      ic595_ledset(LED_CAR_LIGHT_BIT, 1);
      ic595_ledset(LED_LNT_2F_UP_BIT, 0);
      ic595_ledset(LED_CALL_2F_UP_BIT, 0);
      ic595_update();
      servo_door_open();
      break;
    case 3:
      ic595_ledset(LED_CAR_4F_BIT, 1);
      ic595_update();
      break;
    case 4:
      ic595_ledset(LED_CAR_CLOSE_BIT, 1);
      ic595_update();
      servo_door_close();
      ic595_ledset(LED_CAR_CLOSE_BIT, 0);
      state = 5;
      break;
    case 5:
      ic595_ledset(LED_LNT_4F_UP_BIT, 1);
      ic595_update();
      stepper_move_to_floor(3, 2);
      ic595_fndset(3);
      ic595_update();
      stepper_move_to_floor(4, 3);
      ic595_fndset(4);
      ic595_update();
      state = 6;
      ic595_ledset(LED_CAR_4F_BIT, 0);
      ic595_ledset(LED_LNT_4F_UP_BIT, 0);
      ic595_update();
      break;
    case 6:
      servo_door_open();
      break;
    case 7:
      ic595_ledset(LED_CAR_1F_BIT, 1);
      ic595_update();
      _delay_ms(2000);
      state = 8;
      break;
    case 8:
      servo_door_close();
      break;
    case 9:
      servo_door_open();
      _delay_ms(2000);
      state = 10;
      break;
    case 10:
      servo_door_close();
      state = 11;
      break;
    case 11:
      ic595_ledset(LED_LNT_1F_DOWN_BIT, 1);
      ic595_update();
      stepper_move_to_floor(3, 4);
      ic595_fndset(3);
      ic595_update();
      stepper_move_to_floor(2, 3);
      ic595_fndset(2);
      ic595_update();
      stepper_move_to_floor(1, 2);
      ic595_fndset(1);
      ic595_update();
      state = 12;
      break;
    case 12:
      ic595_ledset(LED_CAR_1F_BIT, 0);
      ic595_ledset(LED_LNT_1F_DOWN_BIT, 0);
      ic595_update();
      servo_door_open();
      break;
    case 13:
      ic595_ledset(LED_CAR_OPEN_BIT, 1);
      ic595_update();
      break;
    case 14:
      ic595_ledset(LED_CAR_OPEN_BIT, 0);
      ic595_update();
      servo_door_close();
      _delay_ms(2000);
      ic595_ledset(LED_CAR_LIGHT_BIT, 0);
      ic595_update();
      break;
    }

    ic595_update();
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
  servo_init();

  // HX711 로드셀 핀 설정
  HX711_DT_DDR &= ~(1 << HX711_DT_PIN);  // DT 입력
  HX711_SCK_DDR |= (1 << HX711_SCK_PIN); // SCK 출력
  loadcell_init();
  loadcell_tare();

  // UART 핀 설정
  UART_TX_DDR |= (1 << UART_TX_PIN);  // TX 출력
  UART_RX_DDR &= ~(1 << UART_RX_PIN); // RX 입력

  // 인터럽트 초기화 필요?
  PCMSK1 |= (1 << PCINT12);
  PCICR |= (1 << PCIE1);
  PCIFR |= (1 << PCIF1);
  sei();

  // 초기 출력 상태 업데이트
  ic595_update();
  _delay_ms(100);
}