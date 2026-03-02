; Adizilla OS - Interrupt Handlers
; Hardware interrupt handling
; Built by Lila, Nairobi 🇰🇪

BITS 32

; Interrupt Descriptor Table
IDT_START:

; Divide Error (Division by zero)
idt_0:
    dw 0x08        ; Selector
    dw 0x0000      ; Offset low
    dw 0x8E00      ; Type and attributes
    dw 0x0000      ; Offset high

; Debug Exception
idt_1:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; NMI Interrupt
idt_2:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Breakpoint Exception
idt_3:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Overflow Exception
idt_4:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Bounds Check
idt_5:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Invalid Opcode
idt_6:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Coprocessor Not Available
idt_7:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Double Fault
idt_8:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Coprocessor Segment Overrun
idt_9:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Invalid TSS
idt_10:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Segment Not Present
idt_11:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Stack Fault
idt_12:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; General Protection Fault
idt_13:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Page Fault
idt_14:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Reserved
idt_15:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Math Fault
idt_16:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Alignment Check
idt_17:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Machine Check
idt_18:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; SIMD Floating Point Exception
idt_19:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Virtualization Exception
idt_20:
    dw 0x08
    dw 0x0000
    dw 0x8E00
    dw 0x0000

; Reserved interrupts 21-31
times 11 * 8 db 0

; IRQ0 - Timer
irq_0:
    pushad
    call timer_handler
    popad
    iret

; IRQ1 - Keyboard
irq_1:
    pushad
    call keyboard_handler
    popad
    iret

; IRQ2 - Cascade
irq_2:
    pushad
    popad
    iret

; IRQ3 - COM2
irq_3:
    pushad
    popad
    iret

; IRQ4 - COM1
irq_4:
    pushad
    popad
    iret

; IRQ5 - LPT2
irq_5:
    pushad
    popad
    iret

; IRQ6 - Floppy
irq_6:
    pushad
    popad
    iret

; IRQ7 - LPT1
irq_7:
    pushad
    popad
    iret

; IRQ8 - Real Time Clock
irq_8:
    pushad
    popad
    iret

; IRQ9 - Reserved
irq_9:
    pushad
    popad
    iret

; IRQ10 - Reserved
irq_10:
    pushad
    popad
    iret

; IRQ11 - Reserved
irq_11:
    pushad
    popad
    iret

; IRQ12 - Mouse
irq_12:
    pushad
    popad
    iret

; IRQ13 - Coprocessor
irq_13:
    pushad
    popad
    iret

; IRQ14 - IDE Primary
irq_14:
    pushad
    popad
    iret

; IRQ15 - IDE Secondary
irq_15:
    pushad
    popad
    iret

IDT_END:

; Interrupt handlers
timer_handler:
    ; Handle timer interrupt
    ; Update system clock, process scheduling
    pushf
    cli
    
    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al
    
    popf
    ret

keyboard_handler:
    ; Handle keyboard interrupt
    pushf
    cli
    
    ; Read scan code from keyboard controller
    in al, 0x60
    
    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al
    
    popf
    ret

; Initialize IDT
init_idt:
    ; Load IDT
    lidt [idtr]
    ret

; IDT Register
idtr:
    dw IDT_END - IDT_START - 1  ; Limit
    dd IDT_START                 ; Base

; PIC initialization
init_pic:
    ; ICW1 - Start initialization
    mov al, 0x11
    out 0x20, al
    out 0xA0, al
    
    ; ICW2 - Set interrupt vectors
    mov al, 0x20
    out 0x21, al
    mov al, 0x28
    out 0xA1, al
    
    ; ICW3 - Cascading
    mov al, 0x04
    out 0x21, al
    mov al, 0x02
    out 0xA1, al
    
    ; ICW4 - 8086/88 mode
    mov al, 0x01
    out 0x21, al
    out 0xA1, al
    
    ; Disable all interrupts except timer and keyboard
    mov al, 0xFC
    out 0x21, al
    mov al, 0xFF
    out 0xA1, al
    
    ret
