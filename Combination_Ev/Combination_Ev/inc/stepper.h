#ifndef _STEPPER_H_
#define _STEPPER_H_

#include "pinmacro.h"
#include <stdint.h>

// =================================================================================
// --- 함수 프로토타입 ---
// =================================================================================

/**
 * @brief 스텝모터 초기화
 */
void stepper_init(void);

/**
 * @brief 스텝 패턴을 모터에 출력
 * @param step_pattern 4비트 스텝 패턴
 */
void stepper_step(uint8_t step_pattern);

/**
 * @brief 지정된 스텝 수만큼 모터 이동
 * @param steps 이동할 스텝 수
 * @param direction 방향 (1: 시계방향, 0: 반시계방향)
 */
void stepper_move_steps(int16_t steps, uint8_t direction);

/**
 * @brief 모터를 정확한 각도로 회전
 * @param degrees 회전할 각도 (양수: 시계방향, 음수: 반시계방향)
 */
void stepper_rotate_degrees(int16_t degrees);

/**
 * @brief 모터를 한 바퀴 회전
 * @param direction 방향 (1: 시계방향, 0: 반시계방향)
 */
void stepper_rotate_full(uint8_t direction);

/**
 * @brief 모터 정지 및 전원 차단
 */
void stepper_stop(void);

/**
 * @brief 현재 모터 위치 반환
 * @return 현재 위치 (스텝 단위)
 */
int32_t stepper_get_position(void);

/**
 * @brief 모터 위치 초기화
 */
void stepper_reset_position(void);

/**
 * @brief 엘리베이터를 특정 층으로 이동
 * @param target_floor 목표 층 (1~4)
 * @param current_floor 현재 층 (1~4)
 */
void stepper_move_to_floor(uint8_t target_floor, uint8_t current_floor);

#endif