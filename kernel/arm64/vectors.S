/* AArch64 EL1 vectors: sixteen 128-byte slots, aligned to 2 KiB. */
.section .text.vectors, "ax"
.balign 2048
.global exception_vectors
.type exception_vectors, %function
.macro VECTOR slot
    mov x0, #\slot
    mrs x1, esr_el1
    mrs x2, elr_el1
    mrs x3, far_el1
    b exception_handler
    .space 112
.endm
exception_vectors:
    VECTOR 0
    VECTOR 1
    VECTOR 2
    VECTOR 3
    VECTOR 4
    VECTOR 5
    VECTOR 6
    VECTOR 7
    VECTOR 8
    VECTOR 9
    VECTOR 10
    VECTOR 11
    VECTOR 12
    VECTOR 13
    VECTOR 14
    VECTOR 15
.size exception_vectors, . - exception_vectors
