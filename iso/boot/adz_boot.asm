; Adizilla OS - ISO Bootloader
; Main bootloader for bootable ISO
; Built by Lila, Nairobi 🇰🇪

BITS 16
ORG 0x7C00

; ============================================
; Constants
KERNEL_LOAD_SEG    equ 0x1000
KERNEL_LOAD_OFFSET  equ 0x0000
BOOT_DRIVE         db 0
MSG_LOADING        db 'Adizilla OS v1.0.0 - Loading...', 0
MSG_LOAD_ERROR     db 'Error loading kernel!', 0
MSG_OK            db 'OK', 0

; ============================================
; Main entry point
start:
    ; Save boot drive
    mov [BOOT_DRIVE], dl
    
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
    
    ; Print loading message
    mov si, MSG_LOADING
    call print_string
    
    ; Load kernel from ISO
    call load_kernel
    
    ; Print OK
    mov si, MSG_OK
    call print_string
    call print_newline
    
    ; Jump to kernel
    jmp KERNEL_LOAD_SEG:KERNEL_LOAD_OFFSET

; ============================================
; Print string
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

; ============================================
; Print newline
print_newline:
    mov ah, 0x0E
    mov al, 0x0D
    int 0x10
    mov al, 0x0A
    int 0x10
    ret

; ============================================
; Load kernel from ISO
load_kernel:
    ; Read sectors from disk (simulated - would load from ISO in real implementation)
    ; For ISO, we'd use El Torito specification
    
    ; For now, load a simple test kernel
    mov ah, 0x02        ; BIOS read sector
    mov al, 4           ; Sectors to read
    mov ch, 0           ; Cylinder
    mov cl, 2           ; Sector (boot sector is 1, kernel starts at 2)
    mov dh, 0           ; Head
    mov dl, [BOOT_DRIVE] ; Drive
    mov bx, KERNEL_LOAD_OFFSET ; Buffer
    mov es, bx
    xor bx, bx
    
    int 0x13
    
    jc .load_error
    ret
    
.load_error:
    mov si, MSG_LOAD_ERROR
    call print_string
    jmp $

; ============================================
; Boot signature and padding
times 510-($-$$) db 0
dw 0xAA55

; ============================================
; Additional kernel code that gets loaded
kernel_start:
    ; Simple kernel that prints a message
    mov si, kernel_msg
    call print_string
    jmp $

kernel_msg db 'Adizilla OS Kernel Loaded!', 0

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

; Fill to make 4 sectors
times 2048-($-$$) db 0
