#include "print.h"
#include "app.h"

extern "C" int app() {
    const char *msg = "Anthony\n";
    print(msg);

    __asm__("ecall");

    return 0;
}