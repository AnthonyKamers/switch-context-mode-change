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


.global context_sditch
context_sditch:
    ctx_save    a0  # a0 => save old context
    ctx_load    a1  # a1 => load new context
    ret             # pc=ra => sditch to new process