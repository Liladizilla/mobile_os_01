; Adizilla OS - System Calls
; System call interface for user programs
; Built by Lila, Nairobi 🇰🇪

BITS 32

; System call numbers
%define SYS_exit        1
%define SYS_fork        2
%define SYS_read        3
%define SYS_write       4
%define SYS_open        5
%define SYS_close       6
%define SYS_waitpid     7
%define SYS_creat      8
%define SYS_link       9
%define SYS_unlink     10
%define SYS_execve     11
%define SYS_chdir      12
%define SYS_time       13
%define SYS_mknod      14
%define SYS_chmod      15
%define SYS_chown      16
%define SYS_break      17
%define SYS_stat       18
%define SYS_lseek      19
%define SYS_getpid     20
%define SYS_mount      21
%define SYS_umount     22
%define SYS_setuid     23
%define SYS_getuid     24
%define SYS_stime      25
%define SYS_ptrace     26
%define SYS_alarm      27
%define SYS_fstat      28
%define SYS_pause      29
%define SYS_utime      30
%define SYS_access     33
%define SYS_nice       34
%define SYS_sync       36
%define SYS_kill       37
%define SYS_rename     38
%define SYS_mkdir      39
%define SYS_rmdir      40
%define SYS_dup        41
%define SYS_pipe       42
%define SYS_times      43
%define SYS_prof       44
%define SYS_brk        45
%define SYS_setgid     46
%define SYS_getgid     47
%define SYS_signal     48
%define SYS_geteuid    49
%define SYS_getegid    50
%define SYS_setpgid    51
%define SYS_getppid    52
%define SYS_getpgrp    53
%define SYS_setsid     54
%define SYS_setreuid   55
%define SYS_setregid   56
%define SYS_sigsuspend 63
%define SYS_sigaction  67
%define SYS_sgetmask   68
%define SYS_ssetmask   69
%define SYS_setcore    74
%define SYS_uname       122

; Global syscall table
syscall_table:
    dd sys_exit         ; 1
    dd sys_fork         ; 2
    dd sys_read         ; 3
    dd sys_write        ; 4
    dd sys_open         ; 5
    dd sys_close        ; 6
    dd sys_waitpid      ; 7
    dd sys_creat        ; 8
    dd sys_link         ; 9
    dd sys_unlink       ; 10
    dd sys_execve       ; 11
    dd sys_chdir        ; 12
    dd sys_time         ; 13
    dd sys_mknod        ; 14
    dd sys_chmod        ; 15
    dd sys_chown        ; 16
    dd sys_break        ; 17
    dd sys_stat         ; 18
    dd sys_lseek        ; 19
    dd sys_getpid       ; 20
    dd sys_mount        ; 21
    dd sys_umount       ; 22
    dd sys_setuid       ; 23
    dd sys_getuid       ; 24
    dd sys_stime        ; 25
    dd sys_ptrace      ; 26
    dd sys_alarm        ; 27
    dd sys_fstat        ; 28
    dd sys_pause        ; 29
    dd sys_utime        ; 30
    dd sys_access       ; 33
    dd sys_nice         ; 34
    dd sys_sync         ; 36
    dd sys_kill         ; 37
    dd sys_rename       ; 38
    dd sys_mkdir        ; 39
    dd sys_rmdir        ; 40
    dd sys_dup          ; 41
    dd sys_pipe         ; 42
    dd sys_times        ; 43
    dd sys_prof         ; 44
    dd sys_brk          ; 45
    dd sys_setgid       ; 46
    dd sys_getgid       ; 47
    dd sys_signal       ; 48
    dd sys_geteuid      ; 49
    dd sys_getegid      ; 50
    dd sys_setpgid      ; 51
    dd sys_getppid      ; 52
    dd sys_getpgrp      ; 53
    dd sys_setsid       ; 54
    dd sys_setreuid     ; 55
    dd sys_setregid     ; 56
    dd 0                ; 57 (unused)
    dd 0                ; 58 (unused)
    dd 0                ; 59 (unused)
    dd 0                ; 60 (unused)
    dd 0                ; 61 (unused)
    dd 0                ; 62 (unused)
    dd sys_sigsuspend   ; 63
    dd 0                ; 64 (unused)
    dd 0                ; 65 (unused)
    dd 0                ; 66 (unused)
    dd sys_sigaction    ; 67
    dd sys_sgetmask     ; 68
    dd sys_ssetmask     ; 69

; System call handler
syscall_handler:
    pushad
    
    ; Get syscall number from eax
    mov eax, [esp + 32]  ; Original eax
    
    ; Validate syscall number
    cmp eax, 0
    je .invalid
    cmp eax, 128
    ja .invalid
    
    ; Save registers
    push ebx
    push ecx
    push edx
    
    ; Get syscall function
    shl eax, 2           ; Multiply by 4 for pointer offset
    add eax, syscall_table
    mov eax, [eax]
    
    ; Call syscall
    call eax
    
    ; Restore registers
    pop edx
    pop ecx
    pop ebx
    
    popad
    iret

.invalid:
    mov eax, -1
    popad
    iret

; Stub functions for common syscalls
sys_exit:
    ; Exit system call
    mov eax, 1
    int 0x80
    ret

sys_read:
    ; Read from file descriptor
    mov eax, 3
    int 0x80
    ret

sys_write:
    ; Write to file descriptor
    mov eax, 4
    int 0x80
    ret

sys_open:
    ; Open file
    mov eax, 5
    int 0x80
    ret

sys_close:
    ; Close file
    mov eax, 6
    int 0x80
    ret

sys_getpid:
    ; Get process ID
    mov eax, 20
    ret

sys_fork:
    ; Fork process
    mov eax, 2
    int 0x80
    ret

sys_execve:
    ; Execute program
    mov eax, 11
    int 0x80
    ret

sys_kill:
    ; Send signal to process
    mov eax, 37
    int 0x80
    ret
