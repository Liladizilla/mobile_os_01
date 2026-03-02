; Adizilla OS - Boot Sector
; Initial bootloader for Adizilla OS
; Built by Lila, Nairobi 🇰🇪

BITS 16

; Boot sector start
org 0x7C00

start:
    ; Set up segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Clear screen
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    ; Print boot message
    mov si, boot_msg
    call print
    
    ; Load kernel
    mov si, loading_msg
    call print
    call load_kernel
    
    ; Jump to kernel
    jmp 0x0000:0x1000

print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print
.done:
    ret

load_kernel:
    ; BIOS read disk function
    mov ah, 0x02
    mov al, 10          ; sectors to read
    mov ch, 0           ; cylinder
    mov cl, 2           ; sector
    mov dh, 0           ; head
    mov dl, 0x80        ; first hard disk
    mov bx, 0x1000      ; buffer
    int 0x13
    jc load_error
    ret

load_error:
    mov si, error_msg
    call print
    jmp $

boot_msg db 'Adizilla OS v1.0.0 - Loading...', 13, 10, 0
loading_msg db 'Loading kernel from disk...', 13, 10, 0
error_msg db 'ERROR: Failed to load kernel!', 13, 10, 0

; Fill rest of boot sector
times 510-($-$$) db 0
dw 0xAA55
