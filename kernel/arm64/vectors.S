/*
 * AArch64 EL1 exception vector table.
 * Each architectural slot is 128 bytes; table alignment is 2 KiB.
 * The initial handler reports the syndrome and halts rather than
 * returning to an unknown or potentially corrupted execution state.
 * SPDX-License-Identifier: MIT
 */
.section .text.vectors, "ax"
.balign 2048
.global exception_vectors
.type exception_vectors, %function

.macro VECTOR slot
    mov     x0, #\slot
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, far_el1
    b       exception_handler
    .space  108
.endm

exception_vectors:
    /* Current EL using SP0 */
    VECTOR 0
    VECTOR 1
    VECTOR 2
    VECTOR 3
    /* Current EL using SPx */
    VECTOR 4
    VECTOR 5
    VECTOR 6
    VECTOR 7
    /* Lower EL using AArch64 */
    VECTOR 8
    VECTOR 9
    VECTOR 10
    VECTOR 11
    /* Lower EL using AArch32 */
    VECTOR 12
    VECTOR 13
    VECTOR 14
    VECTOR 15

.size exception_vectors, . - exception_vectors
