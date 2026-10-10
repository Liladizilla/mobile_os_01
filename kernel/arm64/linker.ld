/* QEMU virt direct-kernel load address for the initial Dedus ARM64 target. */
ENTRY(_start)
SECTIONS
{
    . = 0x40080000;
    .text : ALIGN(4096) {
        KEEP(*(.text.boot))
        . = ALIGN(2048);
        __vectors_start = .;
        KEEP(*(.text.vectors))
        . = ALIGN(4096);
        *(.text .text.*)
    }
    .rodata : ALIGN(4096) { *(.rodata .rodata.*) }
    .data : ALIGN(4096) { *(.data .data.*) }
    .bss (NOLOAD) : ALIGN(4096) {
        __bss_start = .;
        *(.bss.stack)
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    __kernel_end = .;
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
