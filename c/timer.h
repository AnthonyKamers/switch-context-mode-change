#include <stdint.h>

#ifndef SWITCH_CONTEXT_MACHINE_MODE_TIMER_H
#define SWITCH_CONTEXT_MACHINE_MODE_TIMER_H

uint64_t timer_handler(uint64_t epc);
void delay(double seconds);

#endif //SWITCH_CONTEXT_MACHINE_MODE_TIMER_H
