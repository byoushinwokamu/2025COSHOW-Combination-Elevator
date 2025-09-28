/*
 * Stepper Motor Test Code - ATmega328P
 * Tests stepper motor control using ULN2003 driver
 */

#include "stepper.h"
#include "pinmacro.h"

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

void test_stepper_basic(void);
void test_stepper_degrees(void);
void test_stepper_floor_movement(void);
void test_stepper_full_rotation(void);

int main(void)
{
    // 스텝모터 초기화
    stepper_init();
    
    _delay_ms(2000); // 초기화 대기
    
    while (1)
    {
        // // === 테스트 1: 기본 스텝 이동 테스트 ===
        // test_stepper_basic();
        // _delay_ms(3000);
        
        // === 테스트 2: 각도 기반 회전 테스트 ===
        test_stepper_degrees();
        _delay_ms(3000);
        
        // // === 테스트 3: 엘리베이터 층간 이동 테스트 ===
        // test_stepper_floor_movement();
        // _delay_ms(3000);
        
        // // === 테스트 4: 전체 회전 테스트 ===
        // test_stepper_full_rotation();
        // _delay_ms(5000);
        
        // 모터 정지 및 위치 리셋
        stepper_stop();
        stepper_reset_position();
        _delay_ms(2000);
    }
}

/**
 * @brief 테스트 1: 기본 스텝 이동 테스트
 * 시계방향과 반시계방향으로 일정 스텝씩 이동합니다.
 */
void test_stepper_basic(void)
{
    // 시계방향으로 512 스텝 이동 (1/8 회전)
    stepper_move_steps(512, 1);
    _delay_ms(1000);
    
    // 반시계방향으로 512 스텝 이동 (원위치)
    stepper_move_steps(512, 0);
    _delay_ms(1000);
    
    // 시계방향으로 1024 스텝 이동 (1/4 회전)
    stepper_move_steps(1024, 1);
    _delay_ms(1000);
    
    // 반시계방향으로 1024 스텝 이동 (원위치)
    stepper_move_steps(1024, 0);
    _delay_ms(1000);
}

/**
 * @brief 테스트 2: 각도 기반 회전 테스트
 * 정확한 각도로 모터를 회전시킵니다.
 */
void test_stepper_degrees(void)
{
    // 45도 시계방향 회전
    stepper_rotate_degrees(45);
    _delay_ms(1000);
    
    // 90도 시계방향 회전 (총 135도)
    stepper_rotate_degrees(90);
    _delay_ms(1000);
    
    // 180도 반시계방향 회전 (총 -45도)
    stepper_rotate_degrees(-180);
    _delay_ms(1000);
    
    // 45도 시계방향 회전 (원위치)
    stepper_rotate_degrees(45);
    _delay_ms(1000);
}

/**
 * @brief 테스트 3: 엘리베이터 층간 이동 테스트
 * 실제 엘리베이터처럼 층간 이동을 시뮬레이션합니다.
 */
void test_stepper_floor_movement(void)
{
    // 1층에서 시작한다고 가정
    uint8_t current_floor = 1;
    
    // 1층 → 2층
    stepper_move_to_floor(2, current_floor);
    current_floor = 2;
    _delay_ms(1500);
    
    // 2층 → 4층
    stepper_move_to_floor(4, current_floor);
    current_floor = 4;
    _delay_ms(1500);
    
    // 4층 → 3층
    stepper_move_to_floor(3, current_floor);
    current_floor = 3;
    _delay_ms(1500);
    
    // 3층 → 1층 (원위치)
    stepper_move_to_floor(1, current_floor);
    current_floor = 1;
    _delay_ms(1500);
}

/**
 * @brief 테스트 4: 전체 회전 테스트
 * 한 바퀴 완전 회전을 테스트합니다.
 */
void test_stepper_full_rotation(void)
{
    // 시계방향 한 바퀴 회전
    stepper_rotate_full(1);
    _delay_ms(2000);
    
    // 반시계방향 한 바퀴 회전 (원위치)
    stepper_rotate_full(0);
    _delay_ms(2000);
}