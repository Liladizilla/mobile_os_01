/* Dedus ARM64 QEMU virt linker layout with explicit permission ranges. */
ENTRY(_start)
SECTIONS
{
    . = 0x40080000;
    __kernel_start = .;
    .text : ALIGN(4096) {
        __text_start = .;
        KEEP(*(.text.boot))
        *(.text .text.[!v]*)
        __text_end = .;
    }
    .vectors ALIGN(2048) : {
        __vectors_start = .;
        KEEP(*(.text.vectors))
        __vectors_end = .;
    }
    .rodata : ALIGN(4096) {
        __rodata_start = .;
        *(.rodata .rodata.*)
        __rodata_end = .;
    }
    .data : ALIGN(4096) {
        __data_start = .;
        *(.data .data.*)
        __data_end = .;
    }
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
