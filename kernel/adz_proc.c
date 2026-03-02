/*
 * Adizilla OS - Process Management
 * Process creation, scheduling, and termination
 * Built by Lila, Nairobi 🇰🇪
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/adz.h"

// Process table
process_t processes[MAX_PROCESSES];
int next_pid = 1;

// Current running process
process_t *current_process = NULL;
process_t *idle_process = NULL;

// Initialize process management
void proc_init(void) {
    memset(processes, 0, sizeof(processes));
    
    // Create idle process
    idle_process = proc_create("idle", NULL, 0);
    if (idle_process) {
        idle_process->priority = 0;
        idle_process->state = PROC_RUNNING;
    }
    
    // Create init process
    proc_create("init", NULL, 0);
    
    printf("[PROC] Process management initialized\n");
}

// Create a new process
process_t *proc_create(const char *name, void *entry, int priority) {
    if (next_pid >= MAX_PROCESSES) {
        printf("[PROC] ERROR: Maximum processes reached\n");
        return NULL;
    }
    
    process_t *proc = &processes[next_pid++];
    
    proc->pid = next_pid - 1;
    strncpy(proc->name, name, MAX_NAME_LEN - 1);
    proc->entry = entry;
    proc->priority = priority;
    proc->state = PROC_READY;
    proc->parent = current_process ? current_process->pid : 0;
    proc->cpu_time = 0;
    proc->memory = 0;
    
    printf("[PROC] Created process %d: %s\n", proc->pid, proc->name);
    
    return proc;
}

// Find process by PID
process_t *proc_find(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == pid && processes[i].state != PROC_DEAD) {
            return &processes[i];
        }
    }
    return NULL;
}

// Terminate a process
void proc_terminate(int pid) {
    process_t *proc = proc_find(pid);
    if (!proc) {
        printf("[PROC] ERROR: Process %d not found\n", pid);
        return;
    }
    
    if (proc->pid == 1) {
        printf("[PROC] Cannot terminate init process\n");
        return;
    }
    
    printf("[PROC] Terminating process %d: %s\n", pid, proc->name);
    proc->state = PROC_DEAD;
}

// Kill all user processes
void proc_kill_all(void) {
    printf("[PROC] Killing all user processes...\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid > 2) {
            processes[i].state = PROC_DEAD;
        }
    }
}

// Schedule next process
void scheduler_tick(void) {
    // Simple round-robin scheduler
    // Find next ready process with highest priority
    process_t *best = NULL;
    int best_priority = -1;
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROC_READY && 
            processes[i].priority > best_priority) {
            best = &processes[i];
            best_priority = processes[i].priority;
        }
    }
    
    if (best) {
        current_process = best;
        current_process->state = PROC_RUNNING;
        current_process->cpu_time++;
    } else {
        current_process = idle_process;
    }
}

// Get process information
void proc_info(process_info_t *info) {
    if (!info) return;
    
    info->count = 0;
    info->running = 0;
    info->sleeping = 0;
    info->zombie = 0;
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state != PROC_DEAD) {
            info->count++;
            switch (processes[i].state) {
                case PROC_RUNNING:
                    info->running++;
                    break;
                case PROC_SLEEPING:
                case PROC_WAITING:
                    info->sleeping++;
                    break;
                case PROC_ZOMBIE:
                    info->zombie++;
                    break;
                default:
                    break;
            }
        }
    }
}

// Sleep a process
void proc_sleep(int ticks) {
    if (!current_process) return;
    
    current_process->state = PROC_SLEEPING;
    current_process->sleep_ticks = ticks;
}

// Wake a process
void proc_wake(int pid) {
    process_t *proc = proc_find(pid);
    if (proc && proc->state == PROC_SLEEPING) {
        proc->state = PROC_READY;
        proc->sleep_ticks = 0;
    }
}

// Get current process
process_t *proc_current(void) {
    return current_process;
}

// List all processes (for ps command)
int proc_list(process_t **list, int max) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES && count < max; i++) {
        if (processes[i].state != PROC_DEAD) {
            list[count++] = &processes[i];
        }
    }
    return count;
}
