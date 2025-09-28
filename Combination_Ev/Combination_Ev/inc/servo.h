/*
 * servo.h - ATmega328P Servo Motor Control Library Header
 * Uses Timer/Counter1 for precise PWM generation on PB1 (OC1A).
 */

#ifndef _SERVO_H_
#define _SERVO_H_

#include "pinmacro.h"
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

// =================================================================================
// --- 상수 정의 ---
// =================================================================================

// =================================================================================
// --- 함수 프로토타입 ---
// =================================================================================

/**
 * @brief 서보 모터 초기화. Timer1을 Fast PWM 모드로 설정합니다.
 */
void servo_init(void);

/**
 * @brief 서보 모터의 목표 각도를 설정합니다.
 * @param angle 설정할 각도 (0 ~ 180도)
 */
void servo_set_angle(uint8_t angle);

/**
 * @brief 엘리베이터 문을 엽니다.
 */
void servo_door_open(void);

/**
 * @brief 엘리베이터 문을 닫습니다.
 */
void servo_door_close(void);

/**
 * @brief 현재 문이 열려있는지 확인합니다.
 * @return 1: 문 열림, 0: 문 닫힘
 */
uint8_t servo_door_is_open(void);

/**
 * @brief 서보 모터를 특정 각도로 부드럽게 이동시킵니다.
 * @param target_angle 목표 각도 (0 ~ 180도)
 * @param step_delay 각 스텝(1도) 사이의 지연시간 (ms)
 */
void servo_move_smooth(uint8_t target_angle, uint16_t step_delay);

#endif /* _SERVO_H_ */