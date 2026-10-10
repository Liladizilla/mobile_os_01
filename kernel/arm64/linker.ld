/* Dedus ARM64 QEMU virt linker layout. */
ENTRY(_start)
SECTIONS
{
    . = 0x40080000;
    .text : ALIGN(4096) {
        KEEP(*(.text.boot))
        *(.text .text.[!v]*)
    }
    .vectors ALIGN(2048) : {
        KEEP(*(.text.vectors))
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
