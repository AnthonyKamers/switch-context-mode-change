#include "config.h"
#include "print.h"

extern "C" uint64 m_trap(uint64 mepc, uint64 mtval, uint64 mcause, uint64 mhart, uint64 mstatus) {
	// O bit mais significativo da causa diz se é uma interrupção ou exceção.
	int interrupt = mcause >> 63 & 1;
	const char *msg = "";

    uint64 return_pc = mepc;
    int cause_num = mcause & 0xffff;

	// Se foi exceção, trata como fatal.
	if (interrupt == 1) {
		msg = "EXCEPTION\n";

        switch(cause_num) {
            case 3:
                msg = "Machine software interrupt\n";
                break;
            case 7:
                msg = "Machine timer interrupt\n";
                break;
            case 11:
                msg = "Machine external interrupt\n";
                break;
            default:
                msg = "Unhandled interrupt\n";
                break;
        }
	} else {
		// Os primeiros dois bits, caso seja interrupção,
        // indicam o modo em que ela aconteceu.
        switch (cause_num) {
            case 1:
                msg = "Failed to access instruction\n";
                break;
            case 2:
                msg = "Illegal Instruction!\n";
                break;
            case 5:
                msg = "Load Access Fault\n";
                break;
            case 8:
                msg = "Call from user-mode\n";
                break;
            case 9:
                msg = "Call from supervisor-mode\n";
                break;
            case 11:
                msg = "Call from machine-mode\n";
                break;
            case 12:
            case 13:
            case 15:
                msg = "Page fault call\n";
                break;
            default:
                msg = "Unhandled call trap\n";
                break;
        }

        return_pc += 4;
	}

	print(msg);
    return return_pc;
}
