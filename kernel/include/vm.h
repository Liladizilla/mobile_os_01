#ifndef DEDUS_VM_H
#define DEDUS_VM_H
#include <stdint.h>
#include <stdbool.h>
uint64_t mmu_kernel_root_address(void);
void mmu_switch_root(uint64_t root_address);
bool vm_self_test(void);
#endif
