#include "print.h"

extern "C" int app(int num) {
    const char *msg = "Anthony\n";
    print(msg);

    return 0;
}