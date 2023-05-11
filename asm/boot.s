.option norvc

.section .text.init
.global _start
_start:
	# Any hardware threads (hart) that are not bootstrapping
	# need to wait for an IPI
	csrr	t0, mhartid
	beqz	t0, halt

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
	li	    t0, (0b01 << 11)
	csrw	mstatus, t0

	# set SUM (Supervisor User Memory access) to 1
	li	    t1, (1 << 18)
	csrw    sstatus, t1

    # make machine trap vector
	la	    t2, asm_trap_vector
	csrw	mtvec, t2

    # enable interruptions (Timer interrupt: 7)
	# li	    t3, (1 << 3) | (1 << 7) | (1 << 11)
	csrw	mie, 0

	# set ALL interruptions and exceptions to supervisor
	li      t4, 0xFFFF
    csrw    mideleg, t4
    csrw    medeleg, t4

    # configure memory protection (necessary to enable supervisor mode) to all memory
    csrw    pmpcfg0, 0b11111

    li      t0, 0xFFFFFFFF
    csrw    pmpaddr0, t0

    # set next function to execute
    la      t0, supervisor_entry
    csrw    mepc, t0

    # configure kernel stack pointer
    jal     init_kernel_stack

	# go to mepc (supervisor_entry)
	mret

supervisor_entry:
    # load mmu
    la      t1, kinit
    csrw    sepc, t1

    # set the return address (from kinit)
    la	    ra, supervisor_setup

    # make supervisor trap vector
    la	    t2, asm_supervisor_trap
    csrw	stvec, t2

    # jump to kinit
    sret

supervisor_setup:
    # kinit returns the table page
    csrw    satp, a0

    # Force the CPU to take our SATP register.
    # To be efficient, if the address space identifier (ASID) portion of SATP is already
    # in cache, it will just grab whatever's in cache. However, that means if we've updated
    # it in memory, it will be the old table. So, sfence.vma will ensure that the MMU always
    # grabs a fresh copy of the SATP register and associated tables.
    sfence.vma

    # 1 << 8  = SPP: Supervisor Previous Privilege (keeps the supervisor mode)
    # 1 << 18 = SUM: Allow Supervisor User Memory access (in order to make new Thread and not new (SYSTEM) Thread) -> This must be changed for other exercises
    li		t0, (1 << 8 | 1 << 18)
    csrw	sstatus, t0

    # init timer
    jal     init_timer

    # configure initial user stack
    jal     init_process

    # configure main as return address
    la		t1, main
    csrw	sepc, t1

    # return to main
    sret

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
