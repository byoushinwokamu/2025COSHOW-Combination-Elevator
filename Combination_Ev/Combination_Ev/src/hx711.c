#include "hx711.h"
<<<<<<< HEAD
#include "pinmacro.h"
#include <util/delay.h>

void hx711_init() {
    HX711_SCK_DDR |= (1 << HX711_SCK_PIN);      // SCK 핀 출력
    HX711_DOUT_DDR &= ~(1 << HX711_DOUT_PIN);   // DOUT 핀 입력
    HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN);    // SCK LOW
}

uint32_t hx711_read() {
    uint32_t data = 0;
    // DOUT이 LOW가 될 때까지 대기
    while (HX711_DOUT_PIN_REG & (1 << HX711_DOUT_PIN));
    // 24비트 데이터 읽기
    for (uint8_t i = 0; i < 24; i++) {
        HX711_SCK_PORT |= (1 << HX711_SCK_PIN);      // SCK HIGH
        _delay_us(1);
        data = (data << 1) | ((HX711_DOUT_PIN_REG & (1 << HX711_DOUT_PIN)) ? 1 : 0);
        HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN);     // SCK LOW
        _delay_us(1);
    }
    // 채널/이득 설정용 추가 클럭 (A채널 128)
    HX711_SCK_PORT |= (1 << HX711_SCK_PIN);
    _delay_us(1);
    HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN);
    _delay_us(1);

    // 24비트 부호 확장
    if (data & 0x800000) data |= 0xFF000000;
    return data;
}

int32_t hx711_read_average(uint8_t times) {
    int64_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        sum += hx711_read();
        _delay_ms(1);
    }
    return (int32_t)(sum / times);
=======
#include <avr/io.h>         // ATmega328P의 레지스터(DDRC, PINC 등) 사용
#include <util/delay.h>     // _delay_ms(), _delay_us() 함수 사용
#include <avr/interrupt.h>  // cli(), sei() 전역 인터럽트 제어 함수 사용

// --- 하드웨어 핀 정의 ---
#define HX711_DT_DDR      DDRC       // 데이터(DT) 핀의 데이터 방향 레지스터
#define HX711_DT_PIN_REG  PINC       // 데이터(DT) 핀의 입력 값 레지스터
#define HX711_DT_PIN      PC0        // 데이터(DT) 핀은 PORTC의 0번 핀
#define HX711_SCK_DDR     DDRC       // 클럭(SCK) 핀의 데이터 방향 레지스터
#define HX711_SCK_PORT    PORTC      // 클럭(SCK) 핀의 출력 값 레지스터
#define HX711_SCK_PIN     PC1        // 클럭(SCK) 핀은 PORTC의 1번 핀

// --- 내부 변수 (static 키워드로 이 파일 안에서만 사용하도록 제한) ---
static long g_offset = 142600;      // 0점(Tare)의 기준이 되는 raw 값 (loadcell_tare 함수로 갱신됨)
static float g_scale = 10.7143;     // raw 값을 무게(kg)로 변환하는 비율 상수 (캘리브레이션 필요!)

// --- 내부 함수 프로토타입 (이 파일 안에서만 사용할 함수이므로 static으로 선언) ---
static long loadcell_read_raw(void);
static bool hx711_is_ready(void);

// --- 외부 공개 함수 구현부 ---

void loadcell_init(void) {
	// SCK 핀은 MCU가 제어하므로 출력, DT 핀은 HX711의 신호를 읽어야 하므로 입력으로 설정
	HX711_SCK_DDR |= (1 << HX711_SCK_PIN);
	HX711_DT_DDR &= ~(1 << HX711_DT_PIN);
	
	// 전원 인가 직후 HX711이 불안정할 수 있으므로, SCK 핀을 제어해 강제로 리셋
	HX711_SCK_PORT |= (1 << HX711_SCK_PIN);  // SCK를 HIGH로 만들어 Power Down 모드 진입
	_delay_us(100);                         // 데이터시트 권장(60us)보다 길게 유지
	HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN);// SCK를 LOW로 내려 다시 깨움(리셋 완료)
}

void loadcell_tare(void) {
	long sum = 0;
	// TARE_SAMPLE_COUNT(10)번 만큼 측정하여 합산
	for (uint8_t i = 0; i < TARE_SAMPLE_COUNT; i++) {
		sum += loadcell_read_raw();
		_delay_ms(10);
	}
	// 평균값을 계산하여 g_offset(0점 기준)으로 저장
	g_offset = sum / TARE_SAMPLE_COUNT;
}

float loadcell_get_weight_g(void) {
	// (현재 raw 값 - 0점 raw 값) / 비율 상수 = 실제 무게(kg)
	return (long)((float)(loadcell_read_raw() - g_offset) / g_scale);
}

bool loadcell_is_overload(void) {
	// 현재 무게가 최대 허용 무게를 초과하는지 여부를 반환
	return (loadcell_get_weight_g() > ELEVATOR_CAPACITY_G);
}


// --- 내부 함수 구현부 ---

// HX711이 데이터를 보낼 준비가 되었는지 확인 (DT 핀이 LOW이면 준비 완료)
static bool hx711_is_ready(void) {
	return !(HX711_DT_PIN_REG & (1 << HX711_DT_PIN));
}

// HX711으로부터 24비트 순수 데이터(raw value)를 읽어오는 저수준 함수
static long loadcell_read_raw(void) {
	long count = 0;
	while (!hx711_is_ready()); // 데이터가 준비될 때까지 기다림
	
	cli(); // 정확한 통신을 위해 다른 인터럽트가 방해하지 못하도록 잠시 중단
	
	// 24번의 클럭 펄스를 발생시켜 1비트씩 데이터를 읽어옴
	for (uint8_t i = 0; i < 24; i++) {
		HX711_SCK_PORT |= (1 << HX711_SCK_PIN);  // 클럭 펄스 시작 (HIGH)
		_delay_us(1);
		count = count << 1;                     // 기존 데이터를 왼쪽으로 1칸 밀어 자리를 만듦
		HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN); // 클럭 펄스 끝 (LOW)
		_delay_us(1);
		if (HX711_DT_PIN_REG & (1 << HX711_DT_PIN)) {
			count++; // DT 핀이 HIGH이면 현재 비트는 1이므로 1을 더함
		}
	}
	
	// 다음 측정을 위한 게인(gain) 값과 채널 설정 펄스 (채널 A, 128배 증폭)
	HX711_SCK_PORT |= (1 << HX711_SCK_PIN);
	_delay_us(1);
	HX711_SCK_PORT &= ~(1 << HX711_SCK_PIN);
	_delay_us(1);
	
	sei(); // 인터럽트 다시 허용
	
	// HX711의 데이터는 2의 보수 형태이므로, 최상위 비트(24번째)가 1이면 음수임
	// 이 경우 32비트 long 자료형에 맞게 음수로 변환해줌
	if (count & 0x800000) {
		count |= 0xFF000000;
	}
	return count;
}

long loadcell_get_raw_value(void) {
	return loadcell_read_raw();
>>>>>>> main
}