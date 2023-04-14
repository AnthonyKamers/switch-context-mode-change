#include <stdint.h>
#include "utils.h"
#include "print.h"
#include "timer.h"

#define TIMER_INTERRUPTION_SECONDS 1
#define MAX_EXECUTION_TIME 10

static int timer_count = 0;

extern "C" void init_timer() {
    volatile uint64* mtimecmp= reinterpret_cast<uint64*>(MTIMECMP);
    volatile uint64* mtime =  reinterpret_cast<uint64*>(MTIME);

    *mtimecmp = *mtime + (TIMER_INTERRUPTION_SECONDS * 10000000);
}

uint64_t timer_handler(uint64_t epc) {
    uint64_t return_pc = epc;

    // disable machine-mode timer interrupts
    set_mie(~((~get_mie()) | MIE_MTIE));

    // print how many times it was scheduled
    const char * message = "Timer count: ";
    print(message, ++timer_count);

    // call schedule function here

    // enable machine-mode timer interrupts
    init_timer();
    set_mie(get_mie() | MIE_MTIE);

    // temporary return_pc
    return return_pc + 4;
}