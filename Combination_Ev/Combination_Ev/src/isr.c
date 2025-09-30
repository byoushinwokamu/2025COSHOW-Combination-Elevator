#include "ic165.h"
#include "ic595.h"
#include "pinmacro.h"
#include "uart.h"

// 외부 함수 선언
extern void stepper_reset_position(void);
extern void set_bell_led_timer(void);
extern void enqueue(uint8_t floor, uint8_t dir);                    // uart.c에 구현됨
extern void handle_external_call(uint8_t floor, uint8_t direction); // main.c에 구현됨
extern volatile uint8_t operation_mode;
extern volatile uint16_t uart_timeout_counter;

// 내부 함수 선언
static uint8_t evaluate_score(uint8_t floor, uint8_t dir);

extern volatile uint16_t swinput;
extern volatile uint16_t door_holding;
extern volatile uint8_t ev_state;
extern volatile uint8_t ev_current_dir;
extern volatile uint8_t ev_current_floor;
extern volatile uint8_t task_queue[5];
volatile uint8_t score_a = 0;
volatile uint8_t score_b = 0;
volatile uint8_t dest_floor = 0;
volatile uint8_t dest_dir = DIR_IDLE;
volatile uint8_t rxbuf = 0;
volatile uint8_t is_req = 0; // 1: Compare to the other E/V, 0: I go

static uint8_t evaluate_score(uint8_t floor, uint8_t dir);

// PC3: Home Sw.
// PC4: Door Closed Sw. (Obstacle Detection)
ISR(PCINT1_vect)
{
  // PC3: 홈 위치 감지 (Active Low)
  if (!(LS_HOME_PIN_REG & (1 << LS_HOME_PIN)))
  {
    // 1층 도달 시 위치 보정
    ev_current_floor = 1;
    // 스텝모터 위치 리셋
    stepper_reset_position();
  }

  // PC4: 장애물 감지 (Active Low)
  if (!(LS_DOOR_CLOSED_PIN_REG & (1 << LS_DOOR_CLOSED_PIN)))
  {
    // 문 닫기 중에만 장애물 감지 처리
    if (ev_state == ST_DOOR_CLOSING)
    {
      ev_state = ST_DOOR_OPENING; // 직접 상태 변경
      door_holding = 0;           // 타이머 리셋
    }
  }
}

// PD5: Any Switch Int. (다이오드 OR 게이트로 연결된 모든 스위치)
ISR(PCINT2_vect)
{
  // 74HC165에서 스위치 상태 읽기 (Active Low)
  uint16_t switch_data = ic165_read();

  // 버튼이 눌렸는지 확인 (0 = 눌림, 1 = 안눌림)
  // 각 버튼에 대해 LOW(0) 상태를 감지

  // 카 내부 버튼들 (Active Low)
  if (!(switch_data & (1 << SW_CAR_OPEN_BIT)))
  {
    ev_state = ST_DOOR_OPENING;
    door_holding = 0;
    // 문 열기 버튼 LED 켜기
    ic595_ledset(LED_CAR_OPEN_BIT, 1);
    ic595_update();
  }
  if (!(switch_data & (1 << SW_CAR_CLOSE_BIT)))
  {
    if (ev_state != ST_DOOR_OPENED) return;
    ev_state = ST_DOOR_CLOSING;
    door_holding = 0;
    // 문 닫기 버튼 LED 켜기
    ic595_ledset(LED_CAR_CLOSE_BIT, 1);
    ic595_update();
    return;
  }
  if (!(switch_data & (1 << SW_CAR_1F_BIT)))
  {
    if (ev_current_dir == DIR_ASCENDING) return;
    dest_floor = 1;
    dest_dir = ev_current_dir;
    is_req = 0;
    // 1층 버튼 LED 켜기
    ic595_ledset(LED_CAR_1F_BIT, 1);
    ic595_update();
  }
  if (!(switch_data & (1 << SW_CAR_2F_BIT)))
  {
    if (ev_current_dir == DIR_ASCENDING && ev_current_floor >= 2) return;
    if (ev_current_dir == DIR_DESCENDING && ev_current_floor <= 2) return;
    dest_floor = 2;
    dest_dir = ev_current_dir;
    is_req = 0;
    // 2층 버튼 LED 켜기
    ic595_ledset(LED_CAR_2F_BIT, 1);
    ic595_update();
  }
  if (!(switch_data & (1 << SW_CAR_3F_BIT)))
  {
    if (ev_current_dir == DIR_ASCENDING && ev_current_floor >= 3) return;
    if (ev_current_dir == DIR_DESCENDING && ev_current_floor <= 3) return;
    dest_floor = 3;
    dest_dir = ev_current_dir;
    is_req = 0;
    // 3층 버튼 LED 켜기
    ic595_ledset(LED_CAR_3F_BIT, 1);
    ic595_update();
  }
  if (!(switch_data & (1 << SW_CAR_4F_BIT)))
  {
    if (ev_current_dir == DIR_DESCENDING) return;
    dest_floor = 4;
    dest_dir = ev_current_dir;
    is_req = 0;
    // 4층 버튼 LED 켜기
    ic595_ledset(LED_CAR_4F_BIT, 1);
    ic595_update();
  }
  if (!(switch_data & (1 << SW_CAR_BELL_BIT)))
  {
    // 벨 버튼 LED 켜기
    ic595_ledset(LED_CAR_BELL_BIT, 1);
    ic595_update();
    set_bell_led_timer(); // 벨 LED 타이머 설정
  }

  // 외부 호출 버튼들 (Active Low)
  if (!(switch_data & (1 << SW_CALL_1F_UP_BIT)))
  {
    dest_floor = 1;
    dest_dir = DIR_ASCENDING;
    is_req = 1;
    // 1층 상행 호출 LED 켜기
    ic595_ledset(LED_CALL_1F_UP_BIT, 1);
    ic595_update();
    handle_external_call(1, DIR_ASCENDING); // 단독/2대 자동 처리
  }
  if (!(switch_data & (1 << SW_CALL_2F_UP_BIT)))
  {
    dest_floor = 2;
    dest_dir = DIR_ASCENDING;
    is_req = 1;
    // 2층 상행 호출 LED 켜기
    ic595_ledset(LED_CALL_2F_UP_BIT, 1);
    ic595_update();
    handle_external_call(2, DIR_ASCENDING);
  }
  if (!(switch_data & (1 << SW_CALL_3F_UP_BIT)))
  {
    dest_floor = 3;
    dest_dir = DIR_ASCENDING;
    is_req = 1;
    // 3층 상행 호출 LED 켜기
    ic595_ledset(LED_CALL_3F_UP_BIT, 1);
    ic595_update();
    handle_external_call(3, DIR_ASCENDING);
  }
  if (!(switch_data & (1 << SW_CALL_2F_DOWN_BIT)))
  {
    dest_floor = 2;
    dest_dir = DIR_DESCENDING;
    is_req = 1;
    // 2층 하행 호출 LED 켜기
    ic595_ledset(LED_CALL_2F_DOWN_BIT, 1);
    ic595_update();
    handle_external_call(2, DIR_DESCENDING);
  }
  if (!(switch_data & (1 << SW_CALL_3F_DOWN_BIT)))
  {
    dest_floor = 3;
    dest_dir = DIR_DESCENDING;
    is_req = 1;
    // 3층 하행 호출 LED 켜기
    ic595_ledset(LED_CALL_3F_DOWN_BIT, 1);
    ic595_update();
    handle_external_call(3, DIR_DESCENDING);
  }
  if (!(switch_data & (1 << SW_CALL_4F_DOWN_BIT)))
  {
    dest_floor = 4;
    dest_dir = DIR_DESCENDING;
    is_req = 1;
    // 4층 하행 호출 LED 켜기
    ic595_ledset(LED_CALL_4F_DOWN_BIT, 1);
    ic595_update();
    handle_external_call(4, DIR_DESCENDING);
  }

  // 내부 버튼 처리 (직접 큐에 추가)
  if (!is_req) // Internal Button Pushed
  {
    enqueue(dest_floor, dest_dir);
  }

  // 현재 스위치 상태 저장 (다음 비교를 위해)
  swinput = switch_data;
}

// UART Receive
ISR(USART_RX_vect)
{
  rxbuf = UDR0; // Read data

  // UART 수신 시 2대 운영 모드로 전환
  operation_mode = 1;
  uart_timeout_counter = 0; // 타이머 리셋

  if (rxbuf & (1 << UART_SENDER_BIT)) // EVB
  {
    score_a = (rxbuf & (0b11100000U)) >> UART_SCORE_BIT;
    score_b = evaluate_score(rxbuf & 0b11, (rxbuf & 0b100) >> 2);
    if (score_a > score_b) enqueue(rxbuf & 0b11, (rxbuf & 0b100) >> 2);
  }
  else // EVA
  {
    if (rxbuf & (1 << UART_ASSIGN)) // returned
      enqueue(rxbuf & 0b11, (rxbuf & 0b100) >> 2);
  }
}

static uint8_t evaluate_score(uint8_t floor, uint8_t dir)
{
  uint8_t score = 0;
  for (uint8_t i = 0; i < 4; i++)
  {
    if (task_queue[i])
    {
      score += 1;
    }
  }
  return score;
}