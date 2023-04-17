.section .text
.global asm_trap_vector
.option norvc
asm_trap_vector:
    # take the return address (which will be the next PC (if timer interrupt)
    mv      s0, ra
    addi    t1, t1, -4      # adjust return address (future PC to the previous instruction)

    jal     switch_kernel_stack
    csrr    s1, satp

    addi    sp, sp, -136
    sd      s0, 0(sp)       # save PC - 4
    sd      s1, 4(sp)       # save SATP
    sd      a0, 16(sp)      # store a0
    sd      a1, 24(sp)      # store a1
    sd      a2, 32(sp)      # store a2
    sd      a3, 40(sp)      # store a3
    sd      a4, 48(sp)      # store a4
    sd      a5, 56(sp)      # store a5
    sd      a6, 64(sp)      # store a6
    sd      a7, 72(sp)      # store a7
    sd      t0, 80(sp)      # store t0
    sd      t1, 88(sp)      # store t1
    sd      t2, 96(sp)      # store t2
    sd      t3, 104(sp)     # store t3
    sd      t4, 112(sp)     # store t4
    sd      t5, 120(sp)     # store t5
    sd      t6, 128(sp)     # store t6

    # old code, used to handle all interruptions
	csrr	a0, mepc	    # Machine exception pc
	csrr	a1, mtval	    # Machine bad address or instruction
	csrr	a2, mcause   	# Machine trap cause
	csrr	a3, mhartid  	# Machine hart id
	csrr	a4, mstatus  	# Machine status
	call	m_trap          # trap handler (in C code (trap.c))

    # m_trap will return the return address via a0
    csrw    mepc, a0

.section .rodata
msg:
	.string "EXCEPTION\n"
