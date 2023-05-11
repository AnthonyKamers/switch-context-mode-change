#define TRUE 1
#define FALSE 0

typedef unsigned long long uint64;

// Machine-mode Interrupt Enable
#define MIE_MEIE (1 << 11) // external
#define MIE_MTIE (1 << 7)  // timer
#define MIE_MSIE (1 << 3)  // software

// Supervisor-mode
#define MPP_S (0b01 << 11) // supervisor