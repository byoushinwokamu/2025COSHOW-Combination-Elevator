/*
 * servo.c - ATmega328P Servo Motor Control Library
 * Uses Timer/Counter1 for precise PWM generation.
 */

// --- CPU 클럭 설정 ---
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "servo.h" // servo_init()등의 함수 프로토타입을 위해 포함

// =================================================================================
// --- 하드웨어 핀 및 상수 정의 ---
// =================================================================================

// 모든 상수는 servo.h에서 정의됨

// =================================================================================
// --- 전역 변수 ---
// =================================================================================
static volatile uint8_t servo_angle = DOOR_CLOSED_ANGLE;

// =================================================================================
// --- 함수 구현 ---
// =================================================================================

/**
 * @brief 서보 모터 초기화. Timer1을 Fast PWM 모드로 설정합니다.
 */
void servo_init(void) {
    // 1. PB1(OC1A) 핀을 출력으로 설정
    SERVO_DDR |= (1 << SERVO_PIN);

    // 2. Fast PWM 모드 (TOP=ICR1, Mode 14) 설정
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM13) | (1 << WGM12);

    // 3. 비반전 모드 설정 (펄스가 OCR1A 값에 따라 길어짐)
    TCCR1A |= (1 << COM1A1);

    // 4. 주기(Period) 설정 (50Hz -> 20ms)
    // 16MHz / 8(prescaler) / 40000(ticks) = 50Hz
    ICR1 = 39999; 

    // 5. 8분주 프리스케일러로 타이머 시작
    TCCR1B |= (1 << CS11);

    // 초기 위치를 문 닫힘 상태로 설정
    servo_set_angle(DOOR_CLOSED_ANGLE);
}

/**
 * @brief 서보 모터의 목표 각도를 설정합니다.
 * @param angle 설정할 각도 (0 ~ 180도)
 */
void servo_set_angle(uint8_t angle) {
    if (angle > 180) {
        angle = 180;
    }
    servo_angle = angle;
    
    // 각도(0-180)를 펄스 폭(1000-2000us)으로 선형 변환
    uint16_t pulse_width_us = SERVO_MIN_PULSE + ((uint32_t)angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE)) / 180;
    
    // 펄스 폭(us)을 타이머 OCR1A 값으로 변환
    // 타이머 클럭 = 16MHz / 8 = 2MHz -> 1틱당 0.5us
    // OCR 값 = 펄스폭(us) / 0.5(us/틱) = 펄스폭 * 2
    OCR1A = pulse_width_us * 2;
}

/**
 * @brief 엘리베이터 문을 엽니다.
 */
void servo_door_open(void) {
    servo_set_angle(DOOR_OPEN_ANGLE);
    _delay_ms(1000); // 문이 완전히 열릴 때까지 대기
}

/**
 * @brief 엘리베이터 문을 닫습니다.
 */
void servo_door_close(void) {
    servo_set_angle(DOOR_CLOSED_ANGLE);
    _delay_ms(1000); // 문이 완전히 닫힐 때까지 대기
}

/**
 * @brief 현재 문이 열려있는지 확인합니다.
 * @return 1: 문 열림, 0: 문 닫힘
 */
uint8_t servo_door_is_open(void) {
    // 45도를 기준으로 열림/닫힘 판단
    return (servo_angle > 45) ? 1 : 0;
}

/**
 * @brief 서보 모터를 특정 각도로 부드럽게 이동시킵니다.
 * @param target_angle 목표 각도
 * @param step_delay 각 스텝(1도) 사이의 지연시간 (ms)
 */
void servo_move_smooth(uint8_t target_angle, uint16_t step_delay) {
    if (target_angle > 180) {
        target_angle = 180;
    }
    
    // 현재 각도와 목표 각도가 같으면 바로 리턴
    if (servo_angle == target_angle) {
        return;
    }
    
    // 현재 각도에서 목표 각도까지 1도씩 이동
    if (target_angle > servo_angle) {
        for (uint8_t angle = servo_angle + 1; angle <= target_angle; angle++) {
            servo_set_angle(angle);
            _delay_ms(step_delay);
        }
    } else {
        for (uint8_t angle = servo_angle - 1; angle >= target_angle && angle <= servo_angle; angle--) {
            servo_set_angle(angle);
            _delay_ms(step_delay);
            if (angle == 0) break;  // uint8_t 언더플로우 방지
        }
    }
}