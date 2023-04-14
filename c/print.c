#include "print.h"

int putchar(int ch) {
	while(uart[UART_REG_TXFIFO] < 0);
	return uart[UART_REG_TXFIFO] = ch & 0xff;
}

// print function for assembly code
extern "C" void print(uint64 a0) {
	const char *s = (char *)(void *)a0;
	while (*s) putchar(*s++);
}

// print function for c code
void print(const char *s) {
	while (*s) putchar(*s++);
}

void print(const char *s, int num) {
    print(s);
    putchar(num + '0');
    print("\n");
}
