# This Code derived from xv6-riscv (64bit)
# -- https://github.com/mit-pdos/xv6-riscv/blob/riscv/kernel/swtch.S

# ============ MACRO ==================
.macro ctx_save base
        sd ra, 0(\base)
        sd sp, 8(\base)
        sd s0, 16(\base)
        sd s1, 24(\base)
        sd s2, 32(\base)
        sd s3, 40(\base)
        sd s4, 48(\base)
        sd s5, 56(\base)
        sd s6, 64(\base)
        sd s7, 72(\base)
        sd s8, 80(\base)
        sd s9, 88(\base)
        sd s10, 96(\base)
        sd s11, 104(\base)
.endm

.macro ctx_load base
        ld ra, 0(\base)
        ld sp, 4(\base)
        ld s0, 8(\base)
        ld s1, 12(\base)
        ld s2, 16(\base)
        ld s3, 20(\base)
        ld s4, 24(\base)
        ld s5, 28(\base)
        ld s6, 32(\base)
        ld s7, 36(\base)
        ld s8, 40(\base)
        ld s9, 44(\base)
        ld s10, 48(\base)
        ld s11, 52(\base)
.endm

# ============ Macro END   ==================


#.global context_switch
#context_switch:
#    ctx_save    a0  # a0 => save old context
#    ctx_load    a1  # a1 => load new context
#    ret             # pc=ra => switch to new process

.global before_context_switch
before_context_switch:
    # kernel stack
    ld      t0, 0(sp)   # load ra (return address from stack)
    addi    sp, sp, 8   # remove space on the stack (pop the ra)
    addi    t0, t0, -4  # adjust ra to previous instruction

    csrr    t1, mepc    # save mepc to t1

    # switch to user stack
    jal     switch_user_stack
    csrr    t2, satp    # read current satp (page table)

    # push info to user stack
    addi    sp, sp, -24
    sd      t0, 0(sp)   # save ra
    sd      t1, 8(sp)   # save mepc
    sd      t2, 16(sp)  # save satp

    # switch to kernel stack (to restore arguments and pop from stack)
    jal     switch_kernel_stack
    ld	 a0, 0(sp)	    # Function argument / return value
    ld	 a1, 8(sp)	    # Function argument / return value
    ld	 a2, 16(sp)	    # Function argument
    ld	 a3, 24(sp)	    # Function argument
    ld	 a4, 32(sp)	    # Function argument
    ld	 a5, 40(sp)	    # Function argument
    ld	 a6, 48(sp)	    # Function argument
    ld	 a7, 56(sp)	    # Function argument
    ld	 t0, 64(sp)	    # Temporary / alternate return address
    ld	 t1, 72(sp)	    # Temporary
    ld	 t2, 80(sp)	    # Temporary
    ld	 t3, 88(sp)	    # Temporary
    ld	 t4, 96(sp)	    # Temporary
    ld	 t5, 104(sp)	# Temporary
    ld	 t6, 112(sp)	# Temporary
    addi    sp, sp, 120

    # switch to user stack (to save the previous context)
    jal     switch_user_stack
    addi 	sp, sp, -128	# Make some space in the stack
    sd 	ra, 0(sp)	# Return address
    sd 	a0, 8(sp)	# Function argument / return value
    sd 	a1, 16(sp)	# Function argument / return value
    sd 	a2, 24(sp)	# Function argument
    sd 	a3, 32(sp)	# Function argument
    sd 	a4, 40(sp)	# Function argument
    sd 	a5, 48(sp)	# Function argument
    sd 	a6, 56(sp)	# Function argument
    sd 	a7, 64(sp)	# Function argument
    sd 	t0, 72(sp)	# Temporary / alternate return address
    sd 	t1, 80(sp)	# Temporary
    sd 	t2, 88(sp)	# Temporary
    sd 	t3, 96(sp)	# Temporary
    sd 	t4, 104(sp)	# Temporary
    sd 	t5, 112(sp)	# Temporary
    sd 	t6, 120(sp)	# Temporary

    j schedule

# a0 = old context
# a1 = new context
.global after_context_switch
after_context_switch:
    # store sp into PCB of old process
    sd      sp, 0(a0)

    # load sp from PCB of new process
    mv      sp, a1
    #ld      sp, 0(a1)
    ld      t0, 128(sp)  # load satp that was saved previously

    # write satp
    csrw    satp, t0

    # pop from the stack
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

    # adjust stack pointer
    addi    sp, sp, 8
    ld      t0, 0(sp)   # load process PC
    jr      t0
