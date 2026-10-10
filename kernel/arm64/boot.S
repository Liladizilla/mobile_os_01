/*
 * Adizilla ARM64 kernel entry point.
 * QEMU virt board, PL011 UART console.
 * SPDX-License-Identifier: MIT
 */
.section .text.boot, "ax"
.global _start
.type _start, %function

_start:
    /* No interrupts before an exception vector and timer are installed. */
    msr     daifset, #0xf
    ldr     x0, =__stack_top
    mov     sp, x0
    bl      kernel_main

1:
    wfe
    b       1b

.size _start, . - _start

.section .bss.stack, "aw", %nobits
.align  12
.global __stack_bottom
__stack_bottom:
    .skip   16384
.global __stack_top
__stack_top:
