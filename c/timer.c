#include <stdint.h>
#include "utils.h"
#include "print.h"
#include "timer.h"

#define TIMER_INTERRUPTION_SECONDS 1
#define MAX_EXECUTION_TIME 10

extern "C" void init_timer() {
    volatile uint64* mtimecmp= reinterpret_cast<uint64*>(MTIMECMP);
    volatile uint64* mtime =  reinterpret_cast<uint64*>(MTIME);

    *mtimecmp = *mtime + (TIMER_INTERRUPTION_SECONDS * 10000000);
}

extern "C" void timer_handler() {
    // disable machine-mode timer interrupts
    set_mie(~((~get_mie()) | MIE_MTIE));

    // enable machine-mode timer interrupts
    set_mie(get_mie() | MIE_MTIE);
    init_timer();

    set_mstatus(get_mstatus() | MIE_MSIE | MPP_S);

    asm("jal before_context_switch");
}