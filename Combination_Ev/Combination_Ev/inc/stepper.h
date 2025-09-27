#ifndef _STEPPER_H_
#define _STEPPER_H_

#include "pinmacro.h"

#include <stdint.h>

void stepper_init();
void stepper_step(uint8_t step_pattern);
void stepper_move_steps(int16_t steps, uint8_t direction);

#endif