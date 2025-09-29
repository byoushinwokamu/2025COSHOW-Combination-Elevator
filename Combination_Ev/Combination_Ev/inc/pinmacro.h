#ifndef _PINMACRO_H_
#define _PINMACRO_H_

#define F_CPU 16000000UL

<<<<<<< HEAD
=======
// E/V Status Macro
#define ST_IDLE 0
#define ST_MOVING 1
#define ST_DOOR_OPENING 2
#define ST_DOOR_OPENED 3
#define ST_DOOR_CLOSING 4

#define DIR_ASCENDING 0
#define DIR_DESCENDING 1
#define DIR_IDLE 2

// UART data bit
#define UART_FLOOR_BIT 0
#define UART_DIRECTION_BIT 2
#define UART_ASSIGN_BIT 3
#define UART_SENDER_BIT 4
#define UART_SCORE_BIT 5
#define UART_ASSIGN 0
#define UART_RETURN 1
#define UART_EVA 0
#define UART_EVB 1

>>>>>>> main
// 74 Series IC Control Pins
#define RCLK_595_DDR DDRB
#define RCLK_595_PORT PORTB
#define RCLK_595_PIN PB2

#define SER_595_DDR DDRB
#define SER_595_PORT PORTB
#define SER_595_PIN PB3

#define MISO_165_DDR DDRB
#define MISO_165_PIN_REG PINB
#define MISO_165_PIN PB4

#define SRCLK_DDR DDRB
#define SRCLK_PORT PORTB
#define SRCLK_PIN PB5

#define CP_LATCH_165_DDR DDRC
#define CP_LATCH_165_PORT PORTC
#define CP_LATCH_165_PIN PC2

// Limit Switch Control Pin
#define LS_HOME_DDR DDRC
#define LS_HOME_PIN_REG PINC
#define LS_HOME_PIN PC3

#define LS_DOOR_OPEN_DDR DDRC
#define LS_DOOR_OPEN_PIN_REG PINC
#define LS_DOOR_OPEN_PIN PC4

#define LS_DOOR_CLOSED_DDR DDRC
#define LS_DOOR_CLOSED_PIN_REG PINC
#define LS_DOOR_CLOSED_PIN PC5

// ULN2003 Stepper Motor Control Pins
#define STEPPER_1_DDR DDRD
#define STEPPER_1_PORT PORTD
#define STEPPER_1_PIN PD4

#define STEPPER_2_DDR DDRD
#define STEPPER_2_PORT PORTD
#define STEPPER_2_PIN PD6

#define STEPPER_3_DDR DDRD
#define STEPPER_3_PORT PORTD
#define STEPPER_3_PIN PD7

#define STEPPER_4_DDR DDRB
#define STEPPER_4_PORT PORTB
#define STEPPER_4_PIN PB0

// Servo Motor Control Pin (PWM)
#define SERVO_DDR DDRB
#define SERVO_PORT PORTB
#define SERVO_PIN PB1

// HX711 Load Cell Control Pins
#define HX711_DT_DDR DDRC
#define HX711_DT_PIN_REG PINC
#define HX711_DT_PIN PC0

#define HX711_SCK_DDR DDRC
#define HX711_SCK_PORT PORTC
#define HX711_SCK_PIN PC1

// IR Tx/Rx Pins
#define IR_TX_DDR DDRB
#define IR_TX_PORT PORTB
#define IR_TX_PIN PB1

#define IR_RX_DDR DDRD
#define IR_RX_PIN_REG PIND
#define IR_RX_PIN PD2

// UART Pins
#define UART_RX_DDR DDRD
#define UART_RX_PIN_REG PIND
#define UART_RX_PIN PD0

#define UART_TX_DDR DDRD
#define UART_TX_PORT PORTD
#define UART_TX_PIN PD1

// 74595 Output Bitmap
// 카 내부 LED
#define LED_CAR_1F_BIT 0
#define LED_CAR_2F_BIT 1
#define LED_CAR_3F_BIT 2
#define LED_CAR_4F_BIT 3
#define LED_CAR_OPEN_BIT 4
#define LED_CAR_CLOSE_BIT 5
#define LED_CAR_BELL_BIT 6
#define LED_CAR_LIGHT_BIT 7
// 외부 호출 LED
#define LED_CALL_1F_UP_BIT 8
#define LED_CALL_2F_UP_BIT 9
#define LED_CALL_2F_DOWN_BIT 10
#define LED_CALL_3F_UP_BIT 11
#define LED_CALL_3F_DOWN_BIT 12
#define LED_CALL_4F_DOWN_BIT 13
// 외부 홀 랜턴 LED
#define LED_LNT_1F_UP_BIT 14
#define LED_LNT_1F_DOWN_BIT 15
#define LED_LNT_2F_UP_BIT 16
#define LED_LNT_2F_DOWN_BIT 17
#define LED_LNT_3F_UP_BIT 18
#define LED_LNT_3F_DOWN_BIT 19
#define LED_LNT_4F_UP_BIT 20
#define LED_LNT_4F_DOWN_BIT 21
// 7-세그먼트 BCD 입력
#define SEG_A_BIT 22
#define SEG_B_BIT 23
#define SEG_C_BIT 24
#define SEG_D_BIT 25

// 74165 Input Bitmap
#define SW_CAR_1F_BIT 0
#define SW_CAR_2F_BIT 1
#define SW_CAR_3F_BIT 2
#define SW_CAR_4F_BIT 3
#define SW_CAR_OPEN_BIT 4
#define SW_CAR_CLOSE_BIT 5
#define SW_CAR_BELL_BIT 6
#define SW_CALL_1F_UP_BIT 7
#define SW_CALL_2F_UP_BIT 8
#define SW_CALL_2F_DOWN_BIT 9
#define SW_CALL_3F_UP_BIT 10
#define SW_CALL_3F_DOWN_BIT 11
#define SW_CALL_4F_DOWN_BIT 12

#endif