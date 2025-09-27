#ifndef _SERVO_H_
#define _SERVO_H_

#include "pinmacro.h"

// 서보 모터 기본 함수
void servo_init();
void servo_set_angle(uint8_t angle);

// 엘리베이터 문 제어 함수
void servo_door_open();
void servo_door_close();
uint8_t servo_door_is_open();
void servo_move_smooth(uint8_t target_angle, uint16_t step_delay);

// 유틸리티 함수
void servo_delay_ms(uint16_t ms);

#endif