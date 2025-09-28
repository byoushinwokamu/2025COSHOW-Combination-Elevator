#ifndef _HX711_H_
#define _HX711_H_

#include "pinmacro.h"

#include <stdint.h>

void hx711_init();
uint32_t hx711_read();
int32_t hx711_read_average(uint8_t times);

#endif