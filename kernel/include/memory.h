#ifndef DEDUS_MEMORY_H
#define DEDUS_MEMORY_H
#include <stdint.h>
#include <stdbool.h>
void memory_init(uint64_t fdt_address);
void memory_print_info(void);
bool memory_self_test(void);
bool memory_is_ready(void);
void uart_write_external(const char *s);
void uart_hex_external(uint64_t value);
#endif
