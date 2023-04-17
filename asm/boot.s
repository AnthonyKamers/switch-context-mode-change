.option norvc

.section .text.init
.global _start
_start:
	# Any hardware threads (hart) that are not bootstrapping
	# need to wait for an IPI
	csrr	t0, mhartid
	bnez	t0, halt

	# SATP should be zero, but let's make sure
	csrw	satp, zero

	# global pointer
	.option push
	.option norelax
	la	    gp, _global_pointer
	.option pop

	# The BSS section is expected to be zero
	la	    a0, _bss_start
	la	    a1, _bss_end
	bgeu	a0, a1, 2f

1:	sd	    zero, (a0)
	addi	a0, a0, 8
	bltu	a0, a1, 1b
2:
	la	    sp, _stack_end

	# We use mret here so that the mstatus register
	# is properly updated.
	li	    t0, (0b11 << 11)
	csrw	mstatus, t0

	# load mmu
	la      t1, kinit
	csrw    mepc, t1

	la	    t2, asm_trap_vector
	csrw	mtvec, t2

	li	    t3, (1 << 3) | (1 << 7) | (1 << 11)
	csrw	mie, t3

    # set the return address (from kinit)
	la	    ra, 3f

	# go to mepc (kinit)
	mret

3:
    # 1 << 3    : Machine Interruption Enable (MIE)
    # 1 << 7    : Machine Previous Interruption Enabled MPIE
    li		t0, (0b11 << 11) | (1 << 7) | (1 << 3)
    csrw	mstatus, t0

    # 1 << 1   : Software interrupt
    # 1 << 5   : Timer interrup
    # 1 << 9   : External interrupt delegated
    # 1 << 7   : Timer Interrupt
    li		t2, (1 << 1) | (1 << 5) | (1 << 9) | (1 << 7)
    csrw	mie, t2

    # configure kernel stack pointer
    jal     init_kernel_stack

    # init timer
    jal     init_timer

    # configure initial user stack
    jal     init_process

    # configure main as return address
    la		t1, main
    csrw	mepc, t1

    # return to main
    mret

# a0 = stack
# a1 = process_entry
# a2 = satp
.global asm_create_process
asm_create_process:
    addi    a0, a0, -136
    sd      a1, 0(a0)       # process entry (PC)
    sd      a2, 8(a0)       # satp
    sd      zero, 16(a0)     # a0
    sd      zero, 24(a0)    # a1
    sd      zero, 32(a0)    # a2
    sd      zero, 40(a0)    # a3
    sd      zero, 48(a0)    # a4
    sd      zero, 56(a0)    # a5
    sd      zero, 64(a0)    # a6
    sd      zero, 72(a0)    # a7
    sd      zero, 80(a0)    # t0
    sd      zero, 88(a0)    # t1
    sd      zero, 96(a0)    # t2
    sd      zero, 104(a0)    # t3
    sd      zero, 112(a0)    # t4
    sd      zero, 120(a0)    # t5
    sd      zero, 128(a0)    # t6
    ret

.global halt
halt:
    wfi
	j	halt
