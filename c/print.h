//
// Created by anthony_kamers on 07/04/23.
//

#ifndef SWITCH_CONTEXT_MACHINE_MODE_PRINT_H
#define SWITCH_CONTEXT_MACHINE_MODE_PRINT_H

typedef unsigned long long uint64;

static const int UART_REG_TXFIFO = 0;
static volatile int *uart = (int *)(void *)0x10000000;

int putchar(int ch);

// print function for assembly code
extern "C" void print(uint64 a0);

// print function for c code
void print(const char *s);


#endif //SWITCH_CONTEXT_MACHINE_MODE_PRINT_H
