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
volatile uint8_t task_queue[5] = {0};
volatile uint8_t target_floor = 0;           // 목표 층
volatile uint8_t emergency_flag = 0;         // 비상 정지 플래그
volatile uint16_t moving_timer = 0;          // 이동 시간 타이머
volatile uint16_t system_timer = 0;          // 시스템 틱 카운터
volatile uint16_t bell_led_timer_global = 0; // 벨 LED 타이머

// 운영 모드 관리
volatile uint8_t operation_mode = 0;        // 0: 단독, 1: 2대 운영
volatile uint16_t uart_timeout_counter = 0; // UART 통신 타임아웃
volatile uint8_t last_uart_received = 0;    // 마지막 UART 수신 시간

// 함수 프로토타입 선언
void init();
void safety_check();
void process_task_queue();
void handle_idle_state();
void handle_moving_state();
void handle_door_opening_state();
void handle_door_opened_state();
void handle_door_closing_state();
void update_display();
uint8_t get_next_task();
void start_moving_to_floor(uint8_t target_floor);
void check_floor_arrival();
void emergency_stop();
void turn_off_car_button_led(uint8_t floor);
void turn_off_call_button_led(uint8_t floor, uint8_t direction);
void init_all_leds();
void set_bell_led_timer();
void check_operation_mode();
void handle_external_call(uint8_t floor, uint8_t direction);

int main(void)
{
  init();

  // 시스템 시작 메시지
  ic595_fndset(ev_current_floor);
  ic595_ledset(LED_CAR_LIGHT_BIT, 1); // 조명 켜기
  ic595_update();

  while (1)
  {
    // 0. 운영 모드 감지
    check_operation_mode();

    // 1. 안전 검사
    safety_check();

    // 2. 작업 큐 처리
    process_task_queue();

    // 3. 상태머신 처리
    switch (ev_state)
    {
    case ST_IDLE:
      handle_idle_state();
      break;

    case ST_MOVING:
      handle_moving_state();
      break;

    case ST_DOOR_OPENING:
      handle_door_opening_state();
      break;

    case ST_DOOR_OPENED:
      handle_door_opened_state();
      break;

    case ST_DOOR_CLOSING:
      handle_door_closing_state();
      break;
    }

    // 4. LED 및 디스플레이 업데이트
    update_display();

    // 5. 시스템 틱 (50ms 주기)
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

  // 리미트 스위치 핀 입력으로 설정 (PC3: Home, PC4: Door Closed만 사용)
  // 외부 풀업 저항이 있으므로, 내부 풀업(PORTC) 설정은 하지 않음
  LS_HOME_DDR &= ~(1 << LS_HOME_PIN);
  LS_DOOR_CLOSED_DDR &= ~(1 << LS_DOOR_CLOSED_PIN);

  // 모든 모듈 초기화
  stepper_init();
  servo_init();
  loadcell_init();
  uart_init(31250); // 31.25kbps

  // 인터럽트 설정
  // PCINT1: PC3(Home), PC4(Door Closed) 인터럽트 활성화
  PCICR |= (1 << PCIE1);
  PCMSK1 |= (1 << PCINT11) | (1 << PCINT12); // PC3 = PCINT11, PC4 = PCINT12

  // PCINT2: PD5 스위치 인터럽트 활성화 (다이오드 OR 게이트 출력)
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT21); // PD5 = PCINT21

  // PD5를 입력으로 설정 (외부 풀업 저항 사용)
  DDRD &= ~(1 << PD5);  // PD5를 입력으로 설정
  PORTD &= ~(1 << PD5); // 내부 풀업 비활성화 (외부 풀업 사용)

  // 전역 인터럽트 활성화
  sei();

  // 모든 LED 초기화 (끄기)
  init_all_leds();

  // 초기 출력 상태 업데이트
  ic595_update();
  _delay_ms(100);

  // 로드셀 영점 조정
  loadcell_tare();
}

// =================================================================================
// 안전 검사 함수
// =================================================================================
void safety_check()
{
  // 과적 감지 - 문이 열려있을 때만 체크
  if (ev_state == ST_DOOR_OPENED)
  {
    if (loadcell_is_overload())
    {
      emergency_flag = 1;
      emergency_stop();
      return;
    }
  }

  // 비상 정지 상태에서 복구 (문이 열려있고 과적이 해제되었을 때)
  if (emergency_flag && ev_state == ST_DOOR_OPENED && !loadcell_is_overload())
  {
    emergency_flag = 0;
    ev_state = ST_DOOR_OPENED; // 문 열림 상태로 복구
    door_holding = 0;          // 타이머 리셋하여 문 열림 시간 연장
  }
}

// =================================================================================
// 작업 큐 처리 함수
// =================================================================================
void process_task_queue()
{
  // 비상 상태에서는 작업 처리 안 함
  if (emergency_flag) return;

  // 대기 상태이고 작업이 있으면 다음 작업 시작
  if (ev_state == ST_IDLE)
  {
    uint8_t next_floor = get_next_task();
    if (next_floor != 0 && next_floor != ev_current_floor)
    {
      start_moving_to_floor(next_floor);
    }
  }
}

// =================================================================================
// IDLE 상태 처리
// =================================================================================
void handle_idle_state()
{
  // 현재 층에 중지 요청이 있는지 확인
  for (uint8_t i = 0; i < 5; i++)
  {
    if (task_queue[i] != 0)
    {
      // uart.c 인코딩 방식에 맞게 수정
      uint8_t floor = (task_queue[i] >> UART_FLOOR_BIT) & 0b11;         // 비트 0-1: 층
      uint8_t direction = (task_queue[i] >> UART_DIRECTION_BIT) & 0b11; // 비트 2-3: 방향
      if (floor == ev_current_floor)
      {
        // 현재 층에 도착 - 관련 LED 끄기
        turn_off_car_button_led(floor);
        turn_off_call_button_led(floor, direction);

        // 문 열기
        ev_state = ST_DOOR_OPENING;
        door_holding = 0;

        // 완료된 작업 제거
        for (uint8_t j = i; j < 4; j++)
        {
          task_queue[j] = task_queue[j + 1];
        }
        task_queue[4] = 0;
        break;
      }
    }
  }
}

// =================================================================================
// 이동 상태 처리
// =================================================================================
void handle_moving_state()
{
  moving_timer++;

  // 이동 시간 초과 감지 (30초 = 600 * 50ms)
  if (moving_timer > 600)
  {
    emergency_stop();
    return;
  }

  // 층 도달 확인
  check_floor_arrival();
}

// =================================================================================
// 문 열기 상태 처리
// =================================================================================
void handle_door_opening_state()
{
  // 서보모터로 문 열기
  servo_door_open();
  ev_state = ST_DOOR_OPENED;
}

// =================================================================================
// 문 열림 상태 처리
// =================================================================================
void handle_door_opened_state()
{
  door_holding++;

  // 과적 상태에서는 문 닫기 금지
  if (emergency_flag || loadcell_is_overload())
  {
    door_holding = 0; // 타이머 리셋하여 계속 열어둠
    return;
  }

  // 문 열림 유지 시간 초과 (DOOR_HOLD_TIME = 1000 * 50ms = 50초)
  if (door_holding >= DOOR_HOLD_TIME)
  {
    ev_state = ST_DOOR_CLOSING;
    door_holding = 0;
  }
}

// =================================================================================
// 문 닫기 상태 처리
// =================================================================================
void handle_door_closing_state()
{
  // 서보모터로 문 닫기
  servo_door_close();
}

// =================================================================================
// 디스플레이 업데이트
// =================================================================================
void update_display()
{
  system_timer++;

  // 방향 LED 표시 (홀 랜턴)
  // 먼저 모든 방향 LED 끄기
  for (uint8_t i = LED_LNT_1F_UP_BIT; i <= LED_LNT_4F_DOWN_BIT; i++)
  {
    ic595_ledset(i, 0);
  }

  // 이동 중일 때만 방향 LED 켜기
  if (ev_state == ST_MOVING)
  {
    if (ev_current_dir == DIR_ASCENDING && ev_current_floor <= 4)
    {
      ic595_ledset(LED_LNT_1F_UP_BIT + (ev_current_floor - 1) * 2, 1);
    }
    else if (ev_current_dir == DIR_DESCENDING && ev_current_floor >= 1)
    {
      ic595_ledset(LED_LNT_1F_DOWN_BIT + (ev_current_floor - 1) * 2, 1);
    }
  }

  // 문 상태 LED - 상태에 따라 자동 제어
  static uint16_t door_led_timer = 0;

  if (ev_state == ST_DOOR_OPENED)
  {
    ic595_ledset(LED_CAR_OPEN_BIT, 1);
    ic595_ledset(LED_CAR_CLOSE_BIT, 0);
  }
  else if (ev_state == ST_DOOR_CLOSING)
  {
    ic595_ledset(LED_CAR_OPEN_BIT, 0);
    ic595_ledset(LED_CAR_CLOSE_BIT, 1);
  }
  else if (ev_state == ST_DOOR_OPENING)
  {
    // 문 열기 중 점멸 효과
    ic595_ledset(LED_CAR_OPEN_BIT, (system_timer % 4 < 2) ? 1 : 0);
    ic595_ledset(LED_CAR_CLOSE_BIT, 0);
  }
  else
  {
    // 일반 상태에서는 버튼을 눌렀을 때만 잠시 켜짐 (3초 후 자동 꺼짐)
    if (door_led_timer > 0)
    {
      door_led_timer--;
      if (door_led_timer == 0)
      {
        ic595_ledset(LED_CAR_OPEN_BIT, 0);
        ic595_ledset(LED_CAR_CLOSE_BIT, 0);
      }
    }
  }

  // 벨 LED 자동 관리
  // 과적/비상 상태에서는 빠른 점멸
  if (emergency_flag || (ev_state == ST_DOOR_OPENED && loadcell_is_overload()))
  {
    ic595_ledset(LED_CAR_BELL_BIT, (system_timer % 4 < 2) ? 1 : 0);
    bell_led_timer_global = 0; // 비상 상태에서는 일반 타이머 리셋
  }
  // 일반 상태에서 벨 버튼을 눌렀을 때 (3초간 점멸 후 꺼짐)
  else if (bell_led_timer_global > 0)
  {
    bell_led_timer_global--;
    // 3초간 점멸 (60 * 50ms)
    ic595_ledset(LED_CAR_BELL_BIT, (system_timer % 6 < 3) ? 1 : 0);
    if (bell_led_timer_global == 0)
    {
      ic595_ledset(LED_CAR_BELL_BIT, 0); // 완전히 끄기
    }
  }
  // 아무것도 없을 때는 끄기
  else
  {
    ic595_ledset(LED_CAR_BELL_BIT, 0);
  }

  // 7-세그먼트 표시
  if (emergency_flag || (ev_state == ST_DOOR_OPENED && loadcell_is_overload()))
  {
    // 과적 시 현재 층 깜빡임 (500ms 주기)
    if (system_timer % 10 < 5)
    {
      ic595_fndset(ev_current_floor); // 현재 층 표시
    }
    else
    {
      ic595_fndset(15); // 빈 표시
    }
  }
  else
  {
    // 정상일 때는 현재 층 표시 + 운영 모드 표시
    if (operation_mode == 0)
    {
      // 단독 운영: 정상 표시
      ic595_fndset(ev_current_floor);
    }
    else
    {
      // 2대 운영: 층 번호에 점 추가 (시각적 구분)
      ic595_fndset(ev_current_floor);
      // 추가적인 LED로 2대 운영 모드 표시 가능 (선택적)
    }
  }

  // 조명 자동 제어 (3초 후 소등)
  if (g_light_timer_count > 0)
  {
    g_light_timer_count--;
    if (g_light_timer_count == 0)
    {
      ic595_ledset(LED_CAR_LIGHT_BIT, 0);
    }
  }

  // 출력 업데이트
  ic595_update();
}

// =================================================================================
// 헬퍼 함수들
// =================================================================================

// 다음 작업 가져오기 (우선순위 기반)
uint8_t get_next_task()
{
  uint8_t best_floor = 0;
  int8_t best_distance = 127; // 최대 거리
  uint8_t best_index = 0;

  for (uint8_t i = 0; i < 5; i++)
  {
    if (task_queue[i] != 0)
    {
      // uart.c 인코딩 방식에 맞게 수정
      uint8_t floor = (task_queue[i] >> UART_FLOOR_BIT) & 0b11;   // 비트 0-1: 층
      uint8_t dir = (task_queue[i] >> UART_DIRECTION_BIT) & 0b11; // 비트 2-3: 방향

      // 거리 계산
      int8_t distance = abs((int8_t)floor - (int8_t)ev_current_floor);

      // 방향 일치 시 우선순위 높음
      if (ev_current_dir != DIR_IDLE)
      {
        if ((ev_current_dir == DIR_ASCENDING && floor > ev_current_floor && dir == DIR_ASCENDING) ||
            (ev_current_dir == DIR_DESCENDING && floor < ev_current_floor && dir == DIR_DESCENDING))
        {
          distance -= 10; // 보너스
        }
      }

      if (distance < best_distance)
      {
        best_distance = distance;
        best_floor = floor;
        best_index = i;
      }
    }
  }

  // 선택된 작업 제거
  if (best_floor != 0)
  {
    for (uint8_t j = best_index; j < 4; j++)
    {
      task_queue[j] = task_queue[j + 1];
    }
    task_queue[4] = 0;
  }

  return best_floor;
}

// 층 이동 시작
void start_moving_to_floor(uint8_t target_floor_param)
{
  if (target_floor_param < 1 || target_floor_param > 4) return;

  target_floor = target_floor_param;
  moving_timer = 0;
  ev_state = ST_MOVING;

  // 방향 결정
  if (target_floor > ev_current_floor)
  {
    ev_current_dir = DIR_ASCENDING;
  }
  else
  {
    ev_current_dir = DIR_DESCENDING;
  }

  // 스텝모터로 이동 시작
  stepper_move_to_floor(target_floor, ev_current_floor);

  // 조명 켜기
  ic595_ledset(LED_CAR_LIGHT_BIT, 1);
  g_light_timer_count = 60; // 3초 (60 * 50ms)
}

// 층 도달 확인
void check_floor_arrival()
{
  // 리미트 스위치로 확인 (홈 위치에서만)
  if (!(LS_HOME_PIN_REG & (1 << LS_HOME_PIN)))
  {
    ev_current_floor = 1;
    stepper_reset_position();
  }

  // 목표 층 도달 확인
  if (ev_current_floor == target_floor)
  {
    ev_state = ST_DOOR_OPENING;
    ev_current_dir = DIR_IDLE;
    door_holding = 0;
    moving_timer = 0;
    stepper_stop();
  }
}

// 비상 정지
void emergency_stop()
{
  ev_state = ST_IDLE;
  ev_current_dir = DIR_IDLE;
  stepper_stop();
  servo_door_close();

  // 모든 LED 끄기
  for (uint8_t i = 0; i < 32; i++)
  {
    ic595_ledset(i, 0);
  }

  // 비상 LED만 켜기
  ic595_ledset(LED_CAR_BELL_BIT, 1);
  ic595_update();
}

// =================================================================================
// LED 제어 헬퍼 함수들
// =================================================================================

// 카 내부 버튼 LED 끄기
void turn_off_car_button_led(uint8_t floor)
{
  if (floor >= 1 && floor <= 4)
  {
    ic595_ledset(LED_CAR_1F_BIT + floor - 1, 0);
  }
}

// 외부 호출 버튼 LED 끄기
void turn_off_call_button_led(uint8_t floor, uint8_t direction)
{
  switch (floor)
  {
  case 1:
    if (direction == DIR_ASCENDING)
    {
      ic595_ledset(LED_CALL_1F_UP_BIT, 0);
    }
    break;
  case 2:
    if (direction == DIR_ASCENDING)
    {
      ic595_ledset(LED_CALL_2F_UP_BIT, 0);
    }
    else if (direction == DIR_DESCENDING)
    {
      ic595_ledset(LED_CALL_2F_DOWN_BIT, 0);
    }
    break;
  case 3:
    if (direction == DIR_ASCENDING)
    {
      ic595_ledset(LED_CALL_3F_UP_BIT, 0);
    }
    else if (direction == DIR_DESCENDING)
    {
      ic595_ledset(LED_CALL_3F_DOWN_BIT, 0);
    }
    break;
  case 4:
    if (direction == DIR_DESCENDING)
    {
      ic595_ledset(LED_CALL_4F_DOWN_BIT, 0);
    }
    break;
  }
}

// 모든 LED 초기화 (끄기)
void init_all_leds()
{
  // 카 내부 버튼 LED 모두 끄기
  for (uint8_t i = LED_CAR_1F_BIT; i <= LED_CAR_4F_BIT; i++)
  {
    ic595_ledset(i, 0);
  }

  // 문 제어 LED 끄기
  ic595_ledset(LED_CAR_OPEN_BIT, 0);
  ic595_ledset(LED_CAR_CLOSE_BIT, 0);

  // 벨 LED 끄기
  ic595_ledset(LED_CAR_BELL_BIT, 0);

  // 외부 호출 LED 모두 끄기
  ic595_ledset(LED_CALL_1F_UP_BIT, 0);
  ic595_ledset(LED_CALL_2F_UP_BIT, 0);
  ic595_ledset(LED_CALL_2F_DOWN_BIT, 0);
  ic595_ledset(LED_CALL_3F_UP_BIT, 0);
  ic595_ledset(LED_CALL_3F_DOWN_BIT, 0);
  ic595_ledset(LED_CALL_4F_DOWN_BIT, 0);

  // 홀 랜턴 LED 모두 끄기
  for (uint8_t i = LED_LNT_1F_UP_BIT; i <= LED_LNT_4F_DOWN_BIT; i++)
  {
    ic595_ledset(i, 0);
  }
}

// 벨 LED 타이머 설정 (isr.c에서 호출 가능)
void set_bell_led_timer()
{
  bell_led_timer_global = 60; // 3초 (60 * 50ms)
}

// =================================================================================
// 운영 모드 관리 함수들
// =================================================================================

// 운영 모드 감지 (단독 vs 2대 운영)
void check_operation_mode()
{
  uart_timeout_counter++;

  // 5초동안 UART 수신이 없으면 단독 모드
  if (uart_timeout_counter > 100)
  { // 100 * 50ms = 5초
    if (operation_mode != 0)
    {
      operation_mode = 0; // 단독 모드
      // 단독 모드 전환 메시지 (선택적)
    }
  }
}

// 외부 호출 처리 (단독/2대 대응)
void handle_external_call(uint8_t floor, uint8_t direction)
{
  if (operation_mode == 0)
  {
    // 단독 운영: 직접 자체 큐에 추가
    enqueue(floor, direction);
  }
  else
  {
    // 2대 운영: UART 통신 + 상황에 따라 자체 처리
    // 스코어 계산 및 전송
    uint8_t my_score = 0;
    for (uint8_t i = 0; i < 5; i++)
    {
      if (task_queue[i] != 0) my_score++;
    }

    uart_tx_data(my_score, ev_current_floor, ev_current_dir, 0);

    // 즐시 자체 큐에도 추가 (백업 처리)
    enqueue(floor, direction);
  }
}