/*
 * ATmega328P 엘리베이터 제어 펌웨어 v3.0
 * Target: ATmega328P @ 16MHz External Crystal
 * 
 * 하드웨어 구성:
 * - 74HC595 시프트 레지스터 (출력): LED 및 7세그먼트 제어
 * - 74HC165 시프트 레지스터 (입력): 버튼/스위치 입력
 * - ULN2003 스텝모터 드라이버: 엘리베이터 상하 이동
 * - 서보모터: 문 개폐 제어
 * - HX711 로드셀: 무게 측정
 * - IR 송수신기: 적외선 통신
 * - UART: 시리얼 통신
 * - 리미트 스위치: 위치 및 문 상태 감지
 * 
 * Author: Updated with pin configuration table
 */

// --- CPU 클럭 설정 ---
#define F_CPU 16000000UL

// --- 헤더 파일 포함 ---
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h> // 인터럽트 사용을 위해 추가


// =================================================================================
// --- 하드웨어 핀 및 비트맵 정의 ---
// =================================================================================

// --- 제어 핀 (Control Pins) ---
#define RCLK_595_DDR    DDRB
#define RCLK_595_PORT   PORTB
#define RCLK_595_PIN    PB2

#define SER_595_DDR     DDRB
#define SER_595_PORT    PORTB
#define SER_595_PIN     PB3

#define MISO_165_DDR    DDRB
#define MISO_165_PIN_REG PINB
#define MISO_165_PIN    PB4

#define SRCLK_DDR       DDRB
#define SRCLK_PORT      PORTB
#define SRCLK_PIN       PB5

#define CP_LATCH_165_DDR      DDRC
#define CP_LATCH_165_PORT     PORTC
#define CP_LATCH_165_PIN      PC2

// --- 리미트 스위치 핀 (Limit Switch Pins) ---
#define LS_HOME_DDR     DDRC
#define LS_HOME_PIN_REG PINC
#define LS_HOME_PIN     PC3

#define LS_DOOR_OPEN_DDR      DDRC
#define LS_DOOR_OPEN_PIN_REG  PINC
#define LS_DOOR_OPEN_PIN      PC4

#define LS_DOOR_CLOSED_DDR    DDRC
#define LS_DOOR_CLOSED_PIN_REG PINC
#define LS_DOOR_CLOSED_PIN    PC5

// --- 스텝모터 제어 핀 (ULN2003 드라이버) ---
#define STEPPER_1_DDR     DDRD
#define STEPPER_1_PORT    PORTD
#define STEPPER_1_PIN     PD4

#define STEPPER_2_DDR     DDRD
#define STEPPER_2_PORT    PORTD
#define STEPPER_2_PIN     PD6

#define STEPPER_3_DDR     DDRD
#define STEPPER_3_PORT    PORTD
#define STEPPER_3_PIN     PD7

#define STEPPER_4_DDR     DDRB
#define STEPPER_4_PORT    PORTB
#define STEPPER_4_PIN     PB0

// --- 서보모터 PWM 제어 핀 ---
#define SERVO_DDR         DDRD
#define SERVO_PORT        PORTD
#define SERVO_PIN         PD3

// --- HX711 로드셀 제어 핀 ---
#define HX711_DT_DDR      DDRC
#define HX711_DT_PIN_REG  PINC
#define HX711_DT_PIN      PC0

#define HX711_SCK_DDR     DDRC
#define HX711_SCK_PORT    PORTC
#define HX711_SCK_PIN     PC1

// --- IR 송수신기 핀 ---
#define IR_TX_DDR         DDRB
#define IR_TX_PORT        PORTB
#define IR_TX_PIN         PB1

#define IR_RX_DDR         DDRD
#define IR_RX_PIN_REG     PIND
#define IR_RX_PIN         PD2

// --- UART 통신 핀 ---
#define UART_RX_DDR       DDRD
#define UART_RX_PIN_REG   PIND
#define UART_RX_PIN       PD0

#define UART_TX_DDR       DDRD
#define UART_TX_PORT      PORTD
#define UART_TX_PIN       PD1

// --- 74HC595 출력 비트맵 (32-bit Output Bitmap) ---
// 카 내부 LED
#define LED_CAR_1F_BIT       0
#define LED_CAR_2F_BIT       1
#define LED_CAR_3F_BIT       2
#define LED_CAR_4F_BIT       3
#define LED_CAR_OPEN_BIT     4
#define LED_CAR_CLOSE_BIT    5
#define LED_CAR_BELL_BIT     6
#define LED_CAR_LIGHT_BIT    7
// 외부 호출 LED
#define LED_CALL_1F_UP_BIT   8
#define LED_CALL_2F_UP_BIT   9
#define LED_CALL_2F_DOWN_BIT 10
#define LED_CALL_3F_UP_BIT   11
#define LED_CALL_3F_DOWN_BIT 12
#define LED_CALL_4F_DOWN_BIT 13
// 외부 홀 랜턴 LED
#define LED_LNT_1F_UP_BIT    14
#define LED_LNT_1F_DOWN_BIT  15
#define LED_LNT_2F_UP_BIT    16
#define LED_LNT_2F_DOWN_BIT  17
#define LED_LNT_3F_UP_BIT    18
#define LED_LNT_3F_DOWN_BIT  19
#define LED_LNT_4F_UP_BIT    20
#define LED_LNT_4F_DOWN_BIT  21
// 7-세그먼트 BCD 입력
#define SEG_A_BIT            22
#define SEG_B_BIT            23
#define SEG_C_BIT            24
#define SEG_D_BIT            25

// --- 74HC165 입력 비트맵 (16-bit Input Bitmap) ---
#define SW_CAR_1F_BIT        0
#define SW_CAR_2F_BIT        1
#define SW_CAR_3F_BIT        2
#define SW_CAR_4F_BIT        3
#define SW_CAR_OPEN_BIT      4
#define SW_CAR_CLOSE_BIT     5
#define SW_CAR_BELL_BIT      6
#define SW_CALL_1F_UP_BIT    7
#define SW_CALL_2F_UP_BIT    8
#define SW_CALL_2F_DOWN_BIT  9
#define SW_CALL_3F_UP_BIT    10
#define SW_CALL_3F_DOWN_BIT  11
#define SW_CALL_4F_DOWN_BIT  12


//=================================================================================
//--- 전역 변수 ---
//=================================================================================
volatile uint32_t g_output_state = 0xFFFFFFFF; // 모든 LED OFF (Active-Low)
volatile uint16_t g_input_state = 0xFFFF;
volatile uint16_t g_light_timer_count = 0; // 3초 조명 타이머를 위한 카운트 변수

// =================================================================================
// --- 함수 프로토타입 ---
// =================================================================================
void update_outputs();
uint16_t read_inputs();
void set_led(uint8_t bit_pos, uint8_t state);
void set_7segment(uint8_t number);

// 스텝모터 제어 함수
void stepper_init();
void stepper_step(uint8_t step_pattern);
void stepper_move_steps(int16_t steps, uint8_t direction);

// 서보모터 제어 함수
void servo_init();
void servo_set_angle(uint8_t angle);

// HX711 로드셀 제어 함수
void hx711_init();
uint32_t hx711_read();
int32_t hx711_read_average(uint8_t times);

// IR 통신 함수
void ir_init();
void ir_transmit(uint8_t data);
uint8_t ir_receive();

// UART 통신 함수
void uart_init(uint16_t baudrate);
void uart_transmit(uint8_t data);
uint8_t uart_receive();
void uart_send_string(const char* str);

// =================================================================================
// --- 메인 함수 ---
// =================================================================================
int main(void) {
    // --- 초기화 ---
    // 시프트 레지스터 제어핀 설정
    RCLK_595_DDR |= (1 << RCLK_595_PIN);
    SER_595_DDR  |= (1 << SER_595_PIN);
    SRCLK_DDR    |= (1 << SRCLK_PIN);
    CP_LATCH_165_DDR   |= (1 << CP_LATCH_165_PIN);
    MISO_165_DDR &= ~(1 << MISO_165_PIN); // MISO는 입력

    // 리미트 스위치 핀 입력으로 설정
    // 외부 풀업 저항이 있으므로, 내부 풀업(PORTC) 설정은 하지 않음
    LS_HOME_DDR &= ~(1 << LS_HOME_PIN);
    LS_DOOR_OPEN_DDR &= ~(1 << LS_DOOR_OPEN_PIN);
    LS_DOOR_CLOSED_DDR &= ~(1 << LS_DOOR_CLOSED_PIN);
    
    // 스텝모터 제어핀 출력 설정
    STEPPER_1_DDR |= (1 << STEPPER_1_PIN);
    STEPPER_2_DDR |= (1 << STEPPER_2_PIN);
    STEPPER_3_DDR |= (1 << STEPPER_3_PIN);
    STEPPER_4_DDR |= (1 << STEPPER_4_PIN);
    
    // 서보모터 PWM 핀 출력 설정
    SERVO_DDR |= (1 << SERVO_PIN);
    
    // HX711 로드셀 핀 설정
    HX711_DT_DDR &= ~(1 << HX711_DT_PIN);     // DT 입력
    HX711_SCK_DDR |= (1 << HX711_SCK_PIN);    // SCK 출력
    
    // IR 송수신기 핀 설정
    IR_TX_DDR |= (1 << IR_TX_PIN);            // IR TX 출력
    IR_RX_DDR &= ~(1 << IR_RX_PIN);           // IR RX 입력
    
    // UART 핀 설정
    UART_TX_DDR |= (1 << UART_TX_PIN);        // TX 출력
    UART_RX_DDR &= ~(1 << UART_RX_PIN);       // RX 입력
    
    // 초기 출력 상태 업데이트
    update_outputs();
    _delay_ms(100);

	 // --- 무한 루프 ---
	 while (1) {
		 // 모든 스위치 상태 읽기
		 g_input_state = read_inputs();
	 
		 // --------------------------------------------------------------------------
		 // --- 버튼 입력 처리 로직 ---
		 // --------------------------------------------------------------------------
	 
		 // 카 내부 1층 버튼
		 if (!(g_input_state & (1 << SW_CAR_1F_BIT))) {
			 set_led(LED_CAR_1F_BIT, 1);
			 set_7segment(1);
		 }
		 // 카 내부 2층 버튼
		 else if (!(g_input_state & (1 << SW_CAR_2F_BIT))) {
			 set_led(LED_CAR_2F_BIT, 1);
			 set_7segment(2);
		 }
		 // 카 내부 3층 버튼
		 else if (!(g_input_state & (1 << SW_CAR_3F_BIT))) {
			 set_led(LED_CAR_3F_BIT, 1);
			 set_7segment(3);
		 }
		 // 카 내부 4층 버튼
		 else if (!(g_input_state & (1 << SW_CAR_4F_BIT))) {
			 set_led(LED_CAR_4F_BIT, 1);
			 set_7segment(4);
		 }
		 // 외부 1층 호출 (UP)
		 else if (!(g_input_state & (1 << SW_CALL_1F_UP_BIT))) {
			 set_led(LED_CALL_1F_UP_BIT, 1);
		 }
		 // 외부 2층 호출 (UP)
		 else if (!(g_input_state & (1 << SW_CALL_2F_UP_BIT))) {
			 set_led(LED_CALL_2F_UP_BIT, 1);
		 }
		 // 외부 2층 호출 (DOWN)
		 else if (!(g_input_state & (1 << SW_CALL_2F_DOWN_BIT))) {
			 set_led(LED_CALL_2F_DOWN_BIT, 1);
		 }
		 // 외부 3층 호출 (UP)
		 else if (!(g_input_state & (1 << SW_CALL_3F_UP_BIT))) {
			 set_led(LED_CALL_3F_UP_BIT, 1);
		 }
		 // 외부 3층 호출 (DOWN)
		 else if (!(g_input_state & (1 << SW_CALL_3F_DOWN_BIT))) {
			 set_led(LED_CALL_3F_DOWN_BIT, 1);
		 }
		 // 외부 4층 호출 (DOWN)
		 else if (!(g_input_state & (1 << SW_CALL_4F_DOWN_BIT))) {
			 set_led(LED_CALL_4F_DOWN_BIT, 1);
		 }
		 // 카 내부 문 열림 버튼
		 else if (!(g_input_state & (1 << SW_CAR_OPEN_BIT))) {
			 set_led(LED_CAR_OPEN_BIT, 1);
		 }
		 // 카 내부 문 닫힘 버튼
		 else if (!(g_input_state & (1 << SW_CAR_CLOSE_BIT))) {
			 set_led(LED_CAR_CLOSE_BIT, 1);
		 }
		 // 카 내부 비상벨 버튼
		 else if (!(g_input_state & (1 << SW_CAR_BELL_BIT))) {
			 set_led(LED_CAR_BELL_BIT, 1);
		 }
		 // 아무 버튼도 눌리지 않았을 경우
		 else {
			 // 모든 층/호출/기능 LED를 끔
			 set_led(LED_CAR_1F_BIT, 0);
			 set_led(LED_CAR_2F_BIT, 0);
			 set_led(LED_CAR_3F_BIT, 0);
			 set_led(LED_CAR_4F_BIT, 0);
			 set_led(LED_CALL_1F_UP_BIT, 0);
			 set_led(LED_CALL_2F_UP_BIT, 0);
			 set_led(LED_CALL_2F_DOWN_BIT, 0);
			 set_led(LED_CALL_3F_UP_BIT, 0);
			 set_led(LED_CALL_3F_DOWN_BIT, 0);
			 set_led(LED_CALL_4F_DOWN_BIT, 0);
			 set_led(LED_CAR_OPEN_BIT, 0);
			 set_led(LED_CAR_CLOSE_BIT, 0);
			 set_led(LED_CAR_BELL_BIT, 0);
			 // (참고: 실제 엘리베이터는 현재 층을 표시해야 하므로,
			 //  이 부분은 나중에 '현재 층' 변수를 받아 처리하도록 수정해야 합니다.)
			 set_7segment(10); // 7세그먼트 끄기
		 }
		 
		 
	     // ★★★★★★★★★★★★★   추가된 조명 제어 로직   ★★★★★★★★★★★★★
	     // --------------------------------------------------------------------------
	     uint8_t should_light_be_on = 0; // 조명을 켤지 결정하는 플래그 변수

	     // 조건 1: 카 내부 버튼이 눌렸는지 확인
	     const uint16_t car_buttons_mask = 0x007F; // 카 내부 버튼(비트 0~6) 마스크
	     if ((g_input_state & car_buttons_mask) != car_buttons_mask) {
		     should_light_be_on = 1; // 켜기
	     }

	     // 조건 2: 리미트 스위치가 눌렸는지 확인 (OR 조건)
	     if (!(LS_HOME_PIN_REG & (1 << LS_HOME_PIN)) ||
	     !(LS_DOOR_OPEN_PIN_REG & (1 << LS_DOOR_OPEN_PIN)) ||
	     !(LS_DOOR_CLOSED_PIN_REG & (1 << LS_DOOR_CLOSED_PIN)))
	     {
		     should_light_be_on = 1; // 켜기
	     }

	     // 최종 결정된 상태에 따라 조명 LED 제어
	     set_led(LED_CAR_LIGHT_BIT, should_light_be_on);
	     // --------------------------------------------------------------------------

		 // 최종 출력 상태를 시프트 레지스터로 전송
		 update_outputs();
	 
		 _delay_ms(50); // 루프 지연
	 }
}

// =================================================================================
// --- 함수 정의 ---
// =================================================================================

// 32비트 g_output_state 값을 74HC595 체인으로 전송
void update_outputs() {
    RCLK_595_PORT &= ~(1 << RCLK_595_PIN); // Latch LOW

    for (uint8_t i = 0; i < 32; i++) {
        if (g_output_state & (1UL << (31 - i))) {
            SER_595_PORT |= (1 << SER_595_PIN);
        } else {
            SER_595_PORT &= ~(1 << SER_595_PIN);
        }
        SRCLK_PORT |= (1 << SRCLK_PIN);
        SRCLK_PORT &= ~(1 << SRCLK_PIN);
    }
    RCLK_595_PORT |= (1 << RCLK_595_PIN); // Latch HIGH
}

// 74HC165 체인에서 16비트 값을 읽어 반환
uint16_t read_inputs() {
    uint16_t data = 0;

    CP_LATCH_165_PORT &= ~(1 << CP_LATCH_165_PIN); // Load LOW
    _delay_us(5);
    CP_LATCH_165_PORT |= (1 << CP_LATCH_165_PIN);  // Load HIGH (데이터 캡처)

    for (uint8_t i = 0; i < 16; i++) {
        data <<= 1;
        if (MISO_165_PIN_REG & (1 << MISO_165_PIN)) {
            data |= 1;
        }
        SRCLK_PORT |= (1 << SRCLK_PIN);
        SRCLK_PORT &= ~(1 << SRCLK_PIN);
    }
    return data;
}

// 특정 LED를 켜거나 끄도록 g_output_state 변수 조작 (Active-Low)
void set_led(uint8_t bit_pos, uint8_t state) {
    if (state) { // 1이면 켜기
        g_output_state &= ~(1UL << bit_pos); // 해당 비트를 0으로
    } else { // 0이면 끄기
        g_output_state |= (1UL << bit_pos);  // 해당 비트를 1로
    }
}


void set_7segment(uint8_t number) {
	// 74LS47은 10 이상의 BCD 코드를 입력받으면 출력을 끄므로,
	// 이를 이용해 디스플레이를 끌 수 있습니다.
	if (number > 9) {
		// 7세그먼트 비트들을 모두 끔 (BCD 15 = Blank)
		g_output_state |= (0b1111UL << SEG_A_BIT);
		return;
	}
	
	// 1. 먼저 7세그먼트 제어 비트(DCBA)를 모두 0으로 깨끗하게 지웁니다.
	g_output_state &= ~(0b1111UL << SEG_A_BIT);
	
	// 2. 그 자리에 원하는 숫자의 BCD 값을 그대로 써넣습니다(OR 연산).
	g_output_state |= ((uint32_t)number << SEG_A_BIT);
}
