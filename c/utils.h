#include <stdint.h>
#include "config.h"

#ifndef SWITCH_CONTEXT_MACHINE_MODE_UTILS_H
#define SWITCH_CONTEXT_MACHINE_MODE_UTILS_H

void set_satp(uint64_t satp_value) {
    asm volatile("csrw satp, a0");
}

uint64_t * get_sp() {
    uint64_t * sp_now;
    asm volatile("mv %0, sp" : "=r"(sp_now));
    return sp_now;
}

uint64_t get_mstatus() {
    uint64_t mstatus_now;
    asm volatile("csrr %0, mstatus" : "=r"(mstatus_now));
    return mstatus_now;
}

void set_sp(uint64_t sp_value) {
    asm volatile("mv sp, a0");
}

uint64_t char_to_satp(char * value) {
    return ((uint64_t)value >> 12)  | (8 << 60);
}

#endif //SWITCH_CONTEXT_MACHINE_MODE_UTILS_H
