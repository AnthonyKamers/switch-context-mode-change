.section .text
.global asm_trap_vector
.option norvc
asm_trap_vector:
    #jal     switch_kernel_stack

    # Save registers
    #addi 	sp, sp, -8	# Make some space in the stack
    #sd 	ra, 0(sp)	# Return address
    #sd 	a0, 8(sp)	# Function argument / return value
    #sd 	a1, 16(sp)	# Function argument / return value
    #sd 	a2, 24(sp)	# Function argument
    #sd 	a3, 32(sp)	# Function argument
    #sd 	a4, 40(sp)	# Function argument
    #sd 	a5, 48(sp)	# Function argument
    #sd 	a6, 56(sp)	# Function argument
    #sd 	a7, 64(sp)	# Function argument
    #sd 	t0, 72(sp)	# Temporary / alternate return address
    #sd 	t1, 80(sp)	# Temporary
    #sd 	t2, 88(sp)	# Temporary
    #sd 	t3, 96(sp)	# Temporary
    #sd 	t4, 104(sp)	# Temporary
    #sd 	t5, 112(sp)	# Temporary
    #sd 	t6, 120(sp)	# Temporary

    #csrr    a0, mepc

    mv      t1, ra
    addi    t1, t1, -4      # adjust return address (future PC to the previous instruction)

    jal     switch_kernel_stack

    jal     timer_handler

    csrr    t0, satp

    addi    sp, sp, -8
    sw      t1, 0(sp)       # save PC - 4
    sw      t0, 4(sp)       # save SATP

    jal     before_context_switch

    #csrrw   t6, mscratch, t6

    #mv      t5, t6
    #csrr    t6, mscratch

	csrr	a0, mepc	    # Machine exception pc
	csrr	a1, mtval	    # Machine bad address or instruction
	csrr	a2, mcause   	# Machine trap cause
	csrr	a3, mhartid  	# Machine hart id
	csrr	a4, mstatus  	# Machine status
	# csrr	a5, t5 	        # Scratch register for machine trap handlers
	# ld      sp, 520(a5)
	call	m_trap          # trap handler (in C code (trap.c))

    # m_trap will return the return address via a0
    csrw    mepc, a0

    # load the trap frame back into t6
    csrr    t6, mscratch

    # restore global pointer registers
    # Restore registers
    ld	 ra, 0(sp)	    # Return address
    ld	 a0, 8(sp)	    # Function argument / return value
    ld	 a1, 16(sp)	    # Function argument / return value
    ld	 a2, 24(sp)	    # Function argument
    ld	 a3, 32(sp)	    # Function argument
    ld	 a4, 40(sp)	    # Function argument
    ld	 a5, 48(sp)	    # Function argument
    ld	 a6, 56(sp)	    # Function argument
    ld	 a7, 64(sp)	    # Function argument
    ld	 t0, 72(sp)	    # Temporary / alternate return address
    ld	 t1, 80(sp)	    # Temporary
    ld	 t2, 88(sp)	    # Temporary
    ld	 t3, 96(sp)	    # Temporary
    ld	 t4, 104(sp)	# Temporary
    ld	 t5, 112(sp)	# Temporary
    ld	 t6, 120(sp)	# Temporary
    addi sp, sp, 128

    # return to "normal" execution (handled by the trap)
    mret
	

.section .rodata
msg:
	.string "EXCEPTION\n"
