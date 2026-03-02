/*
 * Adizilla OS - Core Kernel
 * Main kernel initialization and core functions
 * Built by Lila, Nairobi 🇰🇪
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/adz.h"

// Global kernel state
kernel_state_t kernel_state = {
    .version = "1.0.0",
    .booted = 0,
    .uptime = 0,
    .cpu_count = 0,
    .memory_total = 0,
    .memory_free = 0
};

// Process table
process_t process_table[MAX_PROCESSES];
int process_count = 0;

// Initialize the kernel
void kernel_init(void) {
    printf("[ADZ] Initializing Adizilla OS Kernel v%s\n", kernel_state.version);
    
    // Initialize memory management
    mem_init();
    printf("[ADZ] Memory management initialized\n");
    
    // Initialize process management
    proc_init();
    printf("[ADZ] Process management initialized\n");
    
    // Initialize networking
    net_init();
    printf("[ADZ] Network stack initialized\n");
    
    // Initialize interrupts
    interrupt_init();
    printf("[ADZ] Interrupt handling initialized\n");
    
    // Initialize file system
    vfs_init();
    printf("[ADZ] Virtual file system initialized\n");
    
    kernel_state.booted = 1;
    printf("[ADZ] Kernel boot complete!\n");
}

// Main kernel entry point
void kernel_main(void) {
    kernel_init();
    
    // Start idle process
    printf("[ADZ] Starting init process...\n");
    
    // Main kernel loop
    while (kernel_state.booted) {
        // Handle interrupts
        interrupt_handle();
        
        // Schedule processes
        scheduler_tick();
        
        // Update system time
        kernel_state.uptime++;
    }
}

// Get kernel information
void kernel_info(kernel_info_t *info) {
    if (!info) return;
    
    info->version = kernel_state.version;
    info->uptime = kernel_state.uptime;
    info->cpu_count = kernel_state.cpu_count;
    info->memory_total = kernel_state.memory_total;
    info->memory_free = mem_get_free();
}

// Shutdown the kernel
void kernel_shutdown(void) {
    printf("[ADZ] Shutting down kernel...\n");
    
    // Kill all processes
    proc_kill_all();
    
    // Sync file systems
    vfs_sync();
    
    kernel_state.booted = 0;
    printf("[ADZ] Kernel shutdown complete.\n");
}

// Panic handler
void kernel_panic(const char *message) {
    printf("\n!!! KERNEL PANIC !!!\n");
    printf("Message: %s\n", message);
    printf("Uptime: %lu ticks\n", kernel_state.uptime);
    
    // Dump process table
    printf("\nProcess Table:\n");
    for (int i = 0; i < process_count; i++) {
        printf("  PID %d: %s (state: %d)\n", 
               process_table[i].pid, 
               process_table[i].name,
               process_table[i].state);
    }
    
    // Halt the system
    while (1) {
        __asm__ volatile ("hlt");
    }
}
