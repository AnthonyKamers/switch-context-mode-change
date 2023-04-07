.section .text
.global main
main:
    call app
    li t0, 0
    addi t0, t0, 1
    ret
