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
	# Control registers, set the stack, mstatus, mepc,
	# and mtvec to return to the main function.
	# li	t5, 0xffff;
	# csrw	medeleg, t5
	# csrw	mideleg, t5
	la	    sp, _stack_end

	# We use mret here so that the mstatus register
	# is properly updated.
	# li	t0, (0b11 << 11) | (1 << 7) | (1 << 3)
	li	    t0, (0b11 << 11)
	csrw	mstatus, t0

    # do not allow interruptions
    # csrw mie, zero

    # load main
	# la	t1, main
	# csrw	mepc, t1
	# csrw	sepc, t1

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
    # Setting `sstatus` (supervisor status) register:
    # 1 << 8    : Supervisor's previous protection mode is 1 (SPP=1 [Supervisor]).
    # 1 << 5    : Supervisor's previous interrupt-enable bit is 1 (SPIE=1 [Enabled]).
    # 1 << 1    : Supervisor's interrupt-enable bit will be set to 1 after sret.
    # We set the "previous" bits because the sret will write the current bits
    # with the previous bits.
    li		t0, (0b11 << 11) | (1 << 8) | (1 << 5) | (1 << 7) | (1 << 3)
    csrw	mstatus, t0

    # set the MTIME interval to 1 second (qemu does not implement)
    # li      t0, 0x1000000
    # csrw    mtimecmp, t0

    # Setting `mideleg` (machine interrupt delegate) register:
    # 1 << 1   : Software interrupt delegated to supervisor mode
    # 1 << 5   : Timer interrupt delegated to supervisor mode
    # 1 << 9   : External interrupt delegated to supervisor mode
    # 1 << 7   : Timer Interrupt
    # By default all traps (interrupts or exceptions) automatically
    # cause an elevation to the machine privilege mode (mode 3).
    # When we delegate, we're telling the CPU to only elevate to
    # the supervisor privilege mode (mode 1)
    li		t2, (1 << 1) | (1 << 5) | (1 << 9) | (1 << 7)
    csrw	mideleg, t2

    # Setting `sie` (supervisor interrupt enable) register:
    # This register takes the same bits as mideleg
    # 1 << 1    : Supervisor software interrupt enable (SSIE=1 [Enabled])
    # 1 << 5    : Supervisor timer interrupt enable (STIE=1 [Enabled])
    # 1 << 9    : Supervisor external interrupt enable (SEIE=1 [Enabled])
    csrw	mie, t2

    # Setting `stvec` (supervisor trap vector) register:
    # Essentially this is a function pointer, but the last two bits can be 00 or 01
    # 00        : All exceptions set pc to BASE
    # 01        : Asynchronous interrupts set pc to BASE + 4 x scause
    la		t3, asm_trap_vector
    csrw	mtvec, t3

    # Force the CPU to take our SATP register.
    # To be efficient, if the address space identifier (ASID) portion of SATP is already
    # in cache, it will just grab whatever's in cache. However, that means if we've updated
    # it in memory, it will be the old table. So, sfence.vma will ensure that the MMU always
    # grabs a fresh copy of the SATP register and associated tables.
    sfence.vma

    # memory protection
    #li      t0, 0x90000000
    #srli    t0, t0, 2
    #csrw    pmpaddr0, t0

    #li      t0, 0x0707070F
    #csrw    pmpcfg0, t0

    # kinit() is required to return back the SATP value (including MODE) via a0
    csrw	satp, a0

    # configure kernel stack pointer (and satp)
    jal     init_kernel_stack

    # init timer
    jal     init_timer

    # configure initial user stack
    jal     init_process

    # configure main as return address
    la		t1, main
    csrw	mepc, t1

    # sret will put us in supervisor mode and re-enable interrupts
    mret

# a0 = stack
# a1 = process_entry
# a2 = satp
.global asm_create_process
asm_create_process:
    #addi    a0, a0, -144
    #sd      zero, 0(a0)    # t6
    #sd      zero, 8(a0)    # t5
    #sd      zero, 16(a0)    # t4
    #sd      zero, 24(a0)    # t3
    #sd      zero, 32(a0)    # t2
    #sd      zero, 40(a0)    # t1
    #sd      zero, 48(a0)    # t0
    #sd      zero, 56(a0)    # a7
    #sd      zero, 64(a0)    # a6
    #sd      zero, 72(a0)    # a5
    #sd      zero, 80(a0)    # a4
    #sd      zero, 88(a0)    # a3
    #sd      zero, 96(a0)    # a2
    #sd      zero, 104(a0)    # a1
    #sd      zero, 112(a0)    # a0
    #sd      zero, 120(a0)    # ra
    #sd      a1, 128(a0)       # process entry
    #sd      a2, 136(a0)       # satp
    addi    a0, a0, -4
    sw      a1, 0(a0)       # process entry (PC)
    sw      a2, 4(a0)       # satp
    ret

.global halt
halt:
    wfi
	j	halt
