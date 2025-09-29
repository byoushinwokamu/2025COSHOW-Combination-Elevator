#ifndef LOADCELL_H_
#define LOADCELL_H_

#include "pinmacro.h"

<<<<<<< HEAD
=======
#include <stdbool.h>
>>>>>>> main
#include <stdint.h>

// --- 사용자 설정 ---
// 이 값들을 수정하여 엘리베이터의 사양을 쉽게 변경할 수 있습니다.
#define ELEVATOR_CAPACITY_G 1000L // 엘리베이터 최대 허용 무게 (1000g = 1kg)
#define TARE_SAMPLE_COUNT 10      // 0점 설정 시 10번 측정하여 평균을 내어 정확도를 높입니다.

// =================================================================================
// --- 함수 선언부 ---
// 다른 .c 파일에서 호출하여 사용할 수 있는 함수들의 목록(프로토타입)입니다.
// =================================================================================

/**
 * @brief 로드셀(HX711)의 핀을 초기화하고 안정적인 시작을 위해 리셋 신호를 보냅니다.
 * 프로그램 시작 시 한번만 호출해야 합니다.
 */
void loadcell_init(void);

/**
 * @brief 현재 로드셀의 측정값을 0점(기준점)으로 설정합니다.
 * 엘리베이터가 비어있을 때 호출하여 영점을 맞춥니다.
 */
void loadcell_tare(void);

/**
 * @brief 현재 측정된 무게를 킬로그램(kg) 단위로 변환하여 반환합니다.
 * @return float 현재 무게(kg)
 */
float loadcell_get_weight_g(void);

/**
 * @brief 현재 무게가 설정된 최대 허용 무게(ELEVATOR_CAPACITY_KG)를 초과했는지 확인합니다.
 * @return bool 과적 상태이면 true, 아니면 false를 반환합니다.
 */
bool loadcell_is_overload(void);

// RAW값 반환
long loadcell_get_raw_value(void);

#endif /* LOADCELL_H_ */