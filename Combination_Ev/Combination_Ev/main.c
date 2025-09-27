/*
 * Servo Motor Test Code - ATmega328P
 * Tests servo motor control using Timer1 PWM on PB1 (OC1A)
 */

#include "servo.h"
#include "pinmacro.h"

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

void test_servo_basic(void);
void test_servo_door_control(void);
void test_servo_smooth_movement(void);
void test_servo_sweep(void);

int main(void)
{
    // 서보 모터 초기화
    servo_init();
    
    _delay_ms(2000); // 초기화 대기
    
    while (1)
    {
        // === 테스트 1: 기본 각도 설정 테스트 ===
        test_servo_basic();
        _delay_ms(3000);
        
        // === 테스트 2: 엘리베이터 문 제어 테스트 ===
        test_servo_door_control();
        _delay_ms(3000);
        
        // === 테스트 3: 부드러운 이동 테스트 ===
        test_servo_smooth_movement();
        _delay_ms(3000);
        
        // === 테스트 4: 스위프 테스트 (0도 ~ 180도) ===
        test_servo_sweep();
        _delay_ms(5000);
    }
}

/**
 * @brief 테스트 1: 기본 각도 설정 테스트
 * 0도, 90도, 180도 순서로 서보 각도를 설정합니다.
 */
void test_servo_basic(void)
{
    // 0도 (문 닫힘 위치)
    servo_set_angle(0);
    _delay_ms(1000);
    
    // 90도 (문 열림 위치)
    servo_set_angle(90);
    _delay_ms(1000);
    
    // 180도 (최대 각도)
    servo_set_angle(180);
    _delay_ms(1000);
    
    // 다시 0도로 복귀
    servo_set_angle(0);
    _delay_ms(1000);
}

/**
 * @brief 테스트 2: 엘리베이터 문 제어 테스트
 * 실제 엘리베이터 문 열기/닫기 함수를 테스트합니다.
 */
void test_servo_door_control(void)
{
    // 문 닫기
    servo_door_close();
    _delay_ms(500);
    
    // 문 상태 확인 (닫힘)
    if (!servo_door_is_open()) {
        // 문이 닫혀있음을 시각적으로 확인할 수 있도록 잠시 대기
        _delay_ms(1000);
    }
    
    // 문 열기
    servo_door_open();
    _delay_ms(500);
    
    // 문 상태 확인 (열림)
    if (servo_door_is_open()) {
        // 문이 열려있음을 시각적으로 확인할 수 있도록 잠시 대기
        _delay_ms(1000);
    }
}

/**
 * @brief 테스트 3: 부드러운 이동 테스트
 * 서보가 부드럽게 이동하는지 테스트합니다.
 */
void test_servo_smooth_movement(void)
{
    // 0도에서 90도로 부드럽게 이동 (50ms 간격)
    servo_move_smooth(90, 50);
    _delay_ms(1000);
    
    // 90도에서 180도로 부드럽게 이동 (30ms 간격)
    servo_move_smooth(180, 30);
    _delay_ms(1000);
    
    // 180도에서 0도로 부드럽게 이동 (40ms 간격)
    servo_move_smooth(0, 40);
    _delay_ms(1000);
}

/**
 * @brief 테스트 4: 스위프 테스트
 * 0도부터 180도까지 전체 범위를 스위프합니다.
 */
void test_servo_sweep(void)
{
    uint8_t angle;
    
    // 0도 → 180도 스위프
    for (angle = 0; angle <= 180; angle += 5) {
        servo_set_angle(angle);
        _delay_ms(100);
    }
    
    _delay_ms(500);
    
    // 180도 → 0도 스위프
    for (angle = 180; angle > 0; angle -= 5) {
        servo_set_angle(angle);
        _delay_ms(100);
    }
    servo_set_angle(0); // 마지막에 정확히 0도로 설정
    
    _delay_ms(500);
}