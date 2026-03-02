/*
 * Adizilla OS - Memory Management
 * Physical and virtual memory management
 * Built by Lila, Nairobi 🇰🇪
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/adz.h"

// Memory configuration
#define MEMORY_SIZE        (64 * 1024 * 1024)  // 64 MB
#define PAGE_SIZE          4096
#define PAGE_TABLE_SIZE    (MEMORY_SIZE / PAGE_SIZE / 8)

// Physical memory bitmap
static unsigned char *memory_bitmap;
static unsigned int total_pages;
static unsigned int free_pages;

// Virtual memory areas
static vma_t vma_list[MAX_VMAS];
static int vma_count = 0;

// Initialize memory management
void mem_init(void) {
    total_pages = MEMORY_SIZE / PAGE_SIZE;
    free_pages = total_pages;
    
    // Allocate and clear bitmap
    memory_bitmap = (unsigned char *)calloc(PAGE_TABLE_SIZE, 1);
    if (!memory_bitmap) {
        kernel_panic("Failed to allocate memory bitmap");
    }
    
    // Mark first few pages as used (kernel)
    for (int i = 0; i < 256; i++) {
        mem_set_used(i);
    }
    
    printf("[MEM] Initialized %d KB of memory\n", MEMORY_SIZE / 1024);
}

// Allocate a physical page
void *mem_alloc_page(void) {
    for (unsigned int i = 0; i < total_pages; i++) {
        if (!mem_is_used(i)) {
            mem_set_used(i);
            void *addr = (void *)(i * PAGE_SIZE);
            memset(addr, 0, PAGE_SIZE);
            return addr;
        }
    }
    
    printf("[MEM] WARNING: Out of memory!\n");
    return NULL;
}

// Free a physical page
void mem_free_page(void *addr) {
    unsigned int page = (unsigned int)addr / PAGE_SIZE;
    if (page < total_pages) {
        mem_clear_used(page);
    }
}

// Check if page is used
int mem_is_used(unsigned int page) {
    unsigned int byte = page / 8;
    unsigned int bit = page % 8;
    return memory_bitmap[byte] & (1 << bit);
}

// Set page as used
void mem_set_used(unsigned int page) {
    unsigned int byte = page / 8;
    unsigned int bit = page % 8;
    memory_bitmap[byte] |= (1 << bit);
    free_pages--;
}

// Clear page used flag
void mem_clear_used(unsigned int page) {
    unsigned int byte = page / 8;
    unsigned int bit = page % 8;
    memory_bitmap[byte] &= ~(1 << bit);
    free_pages++;
}

// Get amount of free memory
unsigned int mem_get_free(void) {
    return free_pages * PAGE_SIZE;
}

// Allocate virtual memory
void *mem_vmalloc(size_t size) {
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Find consecutive free pages
    unsigned int start = 0;
    unsigned int count = 0;
    
    for (unsigned int i = 0; i < total_pages; i++) {
        if (!mem_is_used(i)) {
            if (count == 0) start = i;
            count++;
            if (count >= pages) break;
        } else {
            count = 0;
        }
    }
    
    if (count < pages) {
        printf("[MEM] VM allocation failed: not enough memory\n");
        return NULL;
    }
    
    // Mark pages as used
    for (unsigned int i = start; i < start + pages; i++) {
        mem_set_used(i);
    }
    
    return (void *)(start * PAGE_SIZE);
}

// Free virtual memory
void mem_vfree(void *addr, size_t size) {
    unsigned int page = (unsigned int)addr / PAGE_SIZE;
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (size_t i = 0; i < pages; i++) {
        mem_clear_used(page + i);
    }
}

// Copy memory
void *mem_copy(void *dest, const void *src, size_t n) {
    return memcpy(dest, src, n);
}

// Set memory
void *mem_set(void *s, int c, size_t n) {
    return memset(s, c, n);
}

// Memory statistics
void mem_stats(mem_stats_t *stats) {
    if (!stats) return;
    
    stats->total = MEMORY_SIZE;
    stats->used = (total_pages - free_pages) * PAGE_SIZE;
    stats->free = free_pages * PAGE_SIZE;
    stats->largest_block = 0;  // Would require more complex tracking
}
