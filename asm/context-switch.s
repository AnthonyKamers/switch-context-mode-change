.global before_context_switch
before_context_switch:
    # kernel stack
    ld     s0, 0(sp)   # load ra (PC on the next round-robin schedule)
    ld      s1, 8(sp)   # load satp
    ld      a0, 16(sp)   # load a0
    ld      a1, 24(sp)   # load a1
    ld      a2, 32(sp)   # load a2
    ld      a3, 40(sp)   # load a3
    ld      a4, 48(sp)   # load a4
    ld      a5, 56(sp)   # load a5
    ld      a6, 64(sp)   # load a6
    ld      a7, 72(sp)   # load a7
    ld      t0, 80(sp)   # load t0
    ld      t1, 88(sp)   # load t1
    ld      t2, 96(sp)   # load t2
    ld      t3, 104(sp)   # load t3
    ld      t4, 112(sp)   # load t4
    ld      t5, 120(sp)   # load t5
    ld      t6, 128(sp)   # load t6
    addi    sp, sp, 136

    # user stack (push info into that)
    jal     switch_user_stack
    addi    sp, sp, -136
    sd      s0, 0(sp)   # store PC
    sd      s1, 8(sp)   # store satp
    sd      a0, 16(sp)   # store a0
    sd      a1, 24(sp)   # store a1
    sd      a2, 32(sp)   # store a2
    sd      a3, 40(sp)   # store a3
    sd      a4, 48(sp)   # store a4
    sd      a5, 56(sp)   # store a5
    sd      a6, 64(sp)   # store a6
    sd      a7, 72(sp)   # store a7
    sd      t0, 80(sp)   # store t0
    sd      t1, 88(sp)   # store t1
    sd      t2, 96(sp)   # store t2
    sd      t3, 104(sp)   # store t3
    sd      t4, 112(sp)   # store t4
    sd      t5, 120(sp)   # store t5
    sd      t6, 128(sp)   # store t6

    j       schedule

# a0 = old context
# a1 = new context
.global after_context_switch
after_context_switch:
    # store sp into PCB of old process
    sw      sp, 0(a0)

    # load sp from PCB of new process
    mv      sp, a1

    # load PC and other registers
    ld      s0, 0(sp)  # load pc
    ld      s1, 8(sp)  # load satp that was saved previously
    ld      a0, 16(sp)   # load a0
    ld      a1, 24(sp)   # load a1
    ld      a2, 32(sp)   # load a2
    ld      a3, 40(sp)   # load a3
    ld      a4, 48(sp)   # load a4
    ld      a5, 56(sp)   # load a5
    ld      a6, 64(sp)   # load a6
    ld      a7, 72(sp)   # load a7
    ld      t0, 80(sp)   # load t0
    ld      t1, 88(sp)   # load t1
    ld      t2, 96(sp)   # load t2
    ld      t3, 104(sp)   # load t3
    ld      t4, 112(sp)   # load t4
    ld      t5, 120(sp)   # load t5
    ld      t6, 128(sp)   # load t6
    addi    sp, sp, 136

    # write satp
    csrw    satp, s1    # load the new page table

    # go to process function
    jr      s0
