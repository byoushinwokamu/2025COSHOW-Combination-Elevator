/*
 * stepper.c - ATmega328P Stepper Motor Control Library
 * Controls 28BYJ-48 stepper motor with ULN2003 driver
 * Uses 4-step sequence for smooth operation
 */

#include "stepper.h"

// =================================================================================
// --- 상수 정의 ---
// =================================================================================

// 스텝모터 제어 상수
// 28BYJ-48 실제 측정값: Full Step 모드에서 약 2038 스텝/회전
#define STEPS_PER_REVOLUTION 2038 // 28BYJ-48: 실제 측정 기준값 (1바퀴 정확히)
#define STEP_DELAY_MS 5           // 각 스텝 사이의 지연시간 (ms) - 안정적인 동작을 위해 증가

// 스텝 시퀀스 패턴 (Full Step sequence for 28BYJ-48)
// 각 스텝에서 2개의 코일이 동시에 활성화되어 최대 토크 제공
// 방향 수정: 반시계방향 시퀀스로 변경
static const uint8_t step_sequence[4] = {
    0b1001, // Step 0: D+A (코일 4,1 활성화)
    0b0011, // Step 1: C+D (코일 3,4 활성화)
    0b0110, // Step 2: B+C (코일 2,3 활성화)
    0b1100  // Step 3: A+B (코일 1,2 활성화)
};

// 방향 정의
#define STEPPER_DIRECTION_CW 1  // 시계방향
#define STEPPER_DIRECTION_CCW 0 // 반시계방향

// =================================================================================
// --- 전역 변수 ---
// =================================================================================
static volatile int32_t current_position = 0; // 현재 위치 (스텝 단위)
static volatile uint8_t current_step = 0;     // 현재 스텝 패턴 인덱스

// =================================================================================
// --- 함수 구현 ---
// =================================================================================

/**
 * @brief 스텝모터 초기화
 * 스텝모터 제어핀을 출력으로 설정하고 초기 상태로 만듭니다.
 */
void stepper_init(void)
{
  // 스텝모터 제어핀들을 출력으로 설정
  STEPPER_1_DDR |= (1 << STEPPER_1_PIN);
  STEPPER_2_DDR |= (1 << STEPPER_2_PIN);
  STEPPER_3_DDR |= (1 << STEPPER_3_PIN);
  STEPPER_4_DDR |= (1 << STEPPER_4_PIN);

  // 모든 핀을 LOW로 초기화
  STEPPER_1_PORT &= ~(1 << STEPPER_1_PIN);
  STEPPER_2_PORT &= ~(1 << STEPPER_2_PIN);
  STEPPER_3_PORT &= ~(1 << STEPPER_3_PIN);
  STEPPER_4_PORT &= ~(1 << STEPPER_4_PIN);

  // 초기 위치와 스텝 설정
  current_position = 0;
  current_step = 0;
}

/**
 * @brief 스텝 패턴을 모터에 출력합니다.
 * @param step_pattern 4비트 스텝 패턴 (0b0000 ~ 0b1111)
 */
void stepper_step(uint8_t step_pattern)
{
  // 각 비트를 해당 핀에 출력
  if (step_pattern & 0x01)
  {
    STEPPER_1_PORT |= (1 << STEPPER_1_PIN);
  }
  else
  {
    STEPPER_1_PORT &= ~(1 << STEPPER_1_PIN);
  }

  if (step_pattern & 0x02)
  {
    STEPPER_2_PORT |= (1 << STEPPER_2_PIN);
  }
  else
  {
    STEPPER_2_PORT &= ~(1 << STEPPER_2_PIN);
  }

  if (step_pattern & 0x04)
  {
    STEPPER_3_PORT |= (1 << STEPPER_3_PIN);
  }
  else
  {
    STEPPER_3_PORT &= ~(1 << STEPPER_3_PIN);
  }

  if (step_pattern & 0x08)
  {
    STEPPER_4_PORT |= (1 << STEPPER_4_PIN);
  }
  else
  {
    STEPPER_4_PORT &= ~(1 << STEPPER_4_PIN);
  }
}

/**
 * @brief 가변 지연시간을 위한 사용자 정의 지연 함수
 * @param ms 지연시간 (밀리초)
 */
static void delay_ms_variable(uint16_t ms)
{
  while (ms--)
  {
    _delay_ms(1);
  }
}

/**
 * @brief 지정된 스텝 수만큼 모터를 이동시킵니다.
 * @param steps 이동할 스텝 수 (양수: 시계방향, 음수: 반시계방향)
 * @param direction 방향 (1: 시계방향, 0: 반시계방향)
 */
void stepper_move_steps(int16_t steps, uint8_t direction)
{
  uint16_t abs_steps = (steps < 0) ? -steps : steps;

  // 방향이 매개변수로 지정된 경우 steps의 부호 무시
  uint8_t actual_direction = direction;

  for (uint16_t i = 0; i < abs_steps; i++)
  {
    if (actual_direction == STEPPER_DIRECTION_CW)
    {
      // 시계방향: 스텝 시퀀스를 정방향으로
      current_step = (current_step + 1) % 4;
      current_position++;
    }
    else
    {
      // 반시계방향: 스텝 시퀀스를 역방향으로
      current_step = (current_step == 0) ? 3 : current_step - 1;
      current_position--;
    }

    // 현재 스텝 패턴을 모터에 출력
    stepper_step(step_sequence[current_step]);

    // 다음 스텝까지 대기
    delay_ms_variable(STEP_DELAY_MS);
  }
}

/**
 * @brief 모터를 정확한 각도로 회전시킵니다.
 * @param degrees 회전할 각도 (양수: 시계방향, 음수: 반시계방향)
 */
void stepper_rotate_degrees(int16_t degrees)
{
  // 각도를 스텝 수로 변환
  int32_t steps = ((int32_t)degrees * STEPS_PER_REVOLUTION) / 360;

  if (steps >= 0)
  {
    stepper_move_steps(steps, STEPPER_DIRECTION_CW);
  }
  else
  {
    stepper_move_steps(-steps, STEPPER_DIRECTION_CCW);
  }
}

/**
 * @brief 모터를 한 바퀴 회전시킵니다.
 * @param direction 방향 (1: 시계방향, 0: 반시계방향)
 */
void stepper_rotate_full(uint8_t direction)
{
  stepper_move_steps(STEPS_PER_REVOLUTION, direction);
}

/**
 * @brief 모터를 정지시키고 모든 코일의 전원을 끕니다.
 */
void stepper_stop(void)
{
  stepper_step(0x00); // 모든 핀을 LOW로
}

/**
 * @brief 현재 모터의 위치를 반환합니다.
 * @return 현재 위치 (스텝 단위)
 */
int32_t stepper_get_position(void)
{
  return current_position;
}

/**
 * @brief 모터의 위치를 초기화합니다.
 */
void stepper_reset_position(void)
{
  current_position = 0;
  current_step = 0;
}

/**
 * @brief 엘리베이터를 특정 층으로 이동시킵니다.
 * @param target_floor 목표 층 (1~4)
 * @param current_floor 현재 층 (1~4)
 */
void stepper_move_to_floor(uint8_t target_floor, uint8_t current_floor)
{
  if (target_floor < 1 || target_floor > 4 || current_floor < 1 || current_floor > 4)
  {
    return; // 잘못된 층 번호
  }

  if (target_floor == current_floor)
  {
    return; // 이미 목표 층에 있음
  }

  // 층 간 이동에 필요한 스텝 수 (예: 층당 1024 스텝)
  const uint16_t steps_per_floor = STEPS_PER_REVOLUTION / 4;

  int8_t floor_difference = target_floor - current_floor;
  int16_t steps_to_move = floor_difference * steps_per_floor;

  if (steps_to_move > 0)
  {
    stepper_move_steps(steps_to_move, STEPPER_DIRECTION_CW);
  }
  else
  {
    stepper_move_steps(-steps_to_move, STEPPER_DIRECTION_CCW);
  }
}
