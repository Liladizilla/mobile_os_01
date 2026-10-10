/* Dedus ARM64 entry: preserve the boot FDT in .data, normalize EL2, clear BSS. */
.section .text.boot, "ax"
.global _start
.type _start, %function
_start:
    ldr x9, =boot_fdt_pointer
    str x0, [x9]
    msr daifset, #0xf
    mrs x0, CurrentEL
    cmp x0, #0x8
    b.ne 1f
    mov x0, #0x80000000
    msr hcr_el2, x0
    mov x0, #3
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr
    mov x0, #0x3c5
    msr spsr_el2, x0
    adr x0, 1f
    msr elr_el2, x0
    eret
1:
    ldr x0, =__bss_start
    ldr x1, =__bss_end
2:
    cmp x0, x1
    b.hs 3f
    str xzr, [x0], #8
    b 2b
3:
    ldr x0, =__stack_top
    mov sp, x0
    adr x0, exception_vectors
    msr vbar_el1, x0
    isb
    ldr x9, =boot_fdt_pointer
    ldr x0, [x9]
    bl kernel_main
4:
    wfe
    b 4b
.size _start, . - _start
.section .bss.stack, "aw", %nobits
.align 12
.global __stack_bottom
__stack_bottom:
    .skip 16384
.global __stack_top
__stack_top:
.section .data.boot, "aw"
.align 3
.global boot_fdt_pointer
boot_fdt_pointer:
    .quad 0
