.option norvc

.equ STACK_SIZE, 8192

.section .text.init
.global _start
_start:
    # setup stacks per hart
    csrr	t0, mhartid
    slli    t0, t0, 10                  # shift left the hart id by 1024
    la      sp, stacks + STACK_SIZE      # set initial stack pointer to the end of the stack space
    add     sp, sp, t0                  # move the current hart stack pointer to its place in the stack space

	# Any hardware threads (hart) that are not bootstrapping
	# need to wait for an IPI
	csrr    t0, mhartid
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

# clear bss
1:	sd	    zero, (a0)
	addi	a0, a0, 8
	bltu	a0, a1, 1b
2:
	# Control registers, set the stack, mstatus, mepc,
	# and mtvec to return to the main function.
	# li	t5, 0xffff;
	# csrw	medeleg, t5
	# csrw	mideleg, t5
	# la	    sp, _stack_end          # set initial stack pointer

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

    la		t1, main
    csrw	mepc, t1

    # kinit() is required to return back the SATP value (including MODE) via a0
    csrw	satp, a0

    # init timer
    # jal       init_timer

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

    # sret will put us in supervisor mode and re-enable interrupts
    mret

.global halt
halt:
    wfi
	j	halt

stacks:
    .skip STACK_SIZE * 4            # allocate space for the harts stacks
