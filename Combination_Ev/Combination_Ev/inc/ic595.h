#ifndef _IC595_H_
#define _IC595_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "pinmacro.h"

void ic595_update();
void ic595_fndset(uint8_t num);
void ic595_ledset(uint8_t pos, uint8_t state);
uint8_t ic595_fndget();

#endif