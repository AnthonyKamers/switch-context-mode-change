#include <stdint.h>
#include "config.h"

#ifndef SWITCH_CONTEXT_MACHINE_MODE_UTILS_H
#define SWITCH_CONTEXT_MACHINE_MODE_UTILS_H

// registers (not mapped by virt)
#define MTIMECMP 0x02004000
#define MTIME 0x0200bff8

static void set_satp(uint64_t satp_value) {
    asm volatile("csrw satp, a0");
}

static uint64_t char_to_satp(char * value) {
    return ((uint64_t)value >> 12)  | (8 << 60);
}

static uint64_t get_sp() {
    uint64_t sp_now;
    asm volatile("mv %0, sp" : "=r"(sp_now));
    return sp_now;
}

static void set_sp(uint64_t sp_value) {
    asm volatile("mv sp, a0");
}

static uint64_t get_mie() {
    uint64_t mie_now;
    asm volatile("csrr %0, mie" : "=r"(mie_now));
    return mie_now;
}

static void set_mie(uint64_t mie) {
    asm volatile("csrw mie, a0");
}

static uint64_t get_mstatus() {
    uint64_t mstatus_now;
    asm volatile("csrr %0, mstatus" : "=r"(mstatus_now));
    return mstatus_now;
}

#endif //SWITCH_CONTEXT_MACHINE_MODE_UTILS_H
