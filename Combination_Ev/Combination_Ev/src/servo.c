#include <avr/interrupt.h>
#include <avr/io.h>

#include "servo.h"

// 서보 모터 제어 상수
#define SERVO_MIN_PULSE 1000    // 0도 위치 (1ms 펄스)
#define SERVO_MAX_PULSE 2000    // 180도 위치 (2ms 펄스)
#define SERVO_PERIOD 20000      // 20ms 주기 (50Hz)

// 문 위치 정의
#define DOOR_CLOSED_ANGLE 0     // 문 닫힘: 0도
#define DOOR_OPEN_ANGLE 90      // 문 열림: 90도

static volatile uint8_t servo_angle = DOOR_CLOSED_ANGLE;
static volatile uint16_t servo_pulse_width = SERVO_MIN_PULSE;

/**
 * @brief 서보 모터 초기화
 * Timer2를 이용한 PWM 모드 설정 (PD3 핀 - OC2B)
 */
void servo_init() {
    // 서보 모터 핀을 출력으로 설정
    SERVO_DDR |= (1 << SERVO_PIN);
    SERVO_PORT &= ~(1 << SERVO_PIN);
    
    // Timer2 설정 (Fast PWM 모드, 16MHz/64 = 250kHz)
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);  // Fast PWM, Clear OC2B on Compare Match
    TCCR2B = (1 << WGM22) | (1 << CS22);                   // Fast PWM, Prescaler 64
    
    // Timer2 인터럽트 활성화
    TIMSK2 |= (1 << TOIE2);  // Overflow interrupt enable
    
    // 초기 위치를 문 닫힘 상태로 설정
    servo_set_angle(DOOR_CLOSED_ANGLE);
    
    // 전역 인터럽트 활성화
    sei();
}

/**
 * @brief 서보 모터 각도 설정
 * @param angle 설정할 각도 (0~180도)
 */
void servo_set_angle(uint8_t angle) {
    if (angle > 180) angle = 180;
    
    servo_angle = angle;
    
    // 각도를 펄스 폭으로 변환 (1000us ~ 2000us)
    servo_pulse_width = SERVO_MIN_PULSE + ((uint32_t)angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE)) / 180;
    
    // PWM 듀티 사이클 계산 및 설정
    // Timer2는 8비트이므로 적절한 비율로 조정
    uint8_t duty_cycle = (uint8_t)((servo_pulse_width * 255UL) / SERVO_PERIOD);
    OCR2B = duty_cycle;
}

/**
 * @brief 엘리베이터 문 열기
 */
void servo_door_open() {
    servo_set_angle(DOOR_OPEN_ANGLE);
    servo_delay_ms(1000);  // 문이 완전히 열릴 때까지 대기
}

/**
 * @brief 엘리베이터 문 닫기
 */
void servo_door_close() {
    servo_set_angle(DOOR_CLOSED_ANGLE);
    servo_delay_ms(1000);  // 문이 완전히 닫힐 때까지 대기
}

/**
 * @brief 현재 문 상태 확인
 * @return 1: 문 열림, 0: 문 닫힘
 */
uint8_t servo_door_is_open() {
    return (servo_angle > 45) ? 1 : 0;  // 45도 이상이면 열린 것으로 판단
}

/**
 * @brief 서보 모터를 특정 각도로 부드럽게 이동
 * @param target_angle 목표 각도
 * @param step_delay 각 스텝 간 지연시간 (ms)
 */
void servo_move_smooth(uint8_t target_angle, uint16_t step_delay) {
    if (target_angle > 180) target_angle = 180;
    
    int8_t direction = (target_angle > servo_angle) ? 1 : -1;
    
    while (servo_angle != target_angle) {
        servo_angle += direction;
        servo_set_angle(servo_angle);
        servo_delay_ms(step_delay);
    }
}

/**
 * @brief 간단한 밀리초 지연 함수
 * @param ms 지연할 밀리초
 */
void servo_delay_ms(uint16_t ms) {
    volatile uint32_t count;
    for (uint16_t i = 0; i < ms; i++) {
        // 16MHz에서 약 1ms 지연 (대략적인 값)
        for (count = 0; count < 4000; count++) {
            asm volatile ("nop");
        }
    }
}

/**
 * @brief Timer2 오버플로우 인터럽트 서비스 루틴
 * PWM 신호 생성을 위한 소프트웨어 PWM 구현
 */
ISR(TIMER2_OVF_vect) {
    static uint16_t pwm_counter = 0;
    
    pwm_counter++;
    
    // 20ms 주기로 PWM 신호 생성
    if (pwm_counter >= (SERVO_PERIOD / 4)) {  // Timer2 오버플로우 주기 고려
        pwm_counter = 0;
        SERVO_PORT |= (1 << SERVO_PIN);  // HIGH 시작
    }
    
    // 펄스 폭만큼 HIGH 유지 후 LOW
    if (pwm_counter >= (servo_pulse_width / 4)) {
        SERVO_PORT &= ~(1 << SERVO_PIN);  // LOW
    }
}


//test