#ifndef SWITCH_CONTEXT_MACHINE_MODE_PRINT_H
#define SWITCH_CONTEXT_MACHINE_MODE_PRINT_H

#include "config.h"

static const int UART_REG_TXFIFO = 0;
static volatile int *uart = (int *)(void *)0x10010000;

int putchar(int ch);

// print function for assembly code
extern "C" void print(uint64 a0);

// print function for c code
void print(const char *s);
void print(const char *s, int num);


#endif //SWITCH_CONTEXT_MACHINE_MODE_PRINT_H
