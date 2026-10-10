/* Minimal single-page address-space test for the QEMU ARM64 kernel. */
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#include "vm.h"
#define PAGE_SIZE 4096ull
#define USER_TEST_VA 0x1000000000ull /* 64 GiB, outside the kernel identity map */
#define DESC_VALID_TABLE 3ull
#define DESC_AF (1ull<<10)
#define DESC_SH_INNER (3ull<<8)
#define DESC_ATTR_NORMAL (1ull<<2)
#define DESC_AP_EL0_RW (1ull<<6)
#define DESC_PXN (1ull<<53)
#define DESC_UXN (1ull<<54)
static void zero_page(uint64_t address){volatile uint64_t*p=(volatile uint64_t*)(uintptr_t)address;for(unsigned i=0;i<512;i++)p[i]=0;}
bool vm_self_test(void){
    uint64_t root_pa=memory_alloc_page();
    uint64_t l2_pa=memory_alloc_page();
    uint64_t l3_pa=memory_alloc_page();
    uint64_t data_pa=memory_alloc_page();
    if(!root_pa||!l2_pa||!l3_pa||!data_pa){
        if(root_pa)memory_free_page(root_pa);
        if(l2_pa)memory_free_page(l2_pa);
        if(l3_pa)memory_free_page(l3_pa);
        if(data_pa)memory_free_page(data_pa);
        return false;
    }
    uint64_t*root=(uint64_t*)(uintptr_t)root_pa;
    uint64_t*l2=(uint64_t*)(uintptr_t)l2_pa;
    uint64_t*l3=(uint64_t*)(uintptr_t)l3_pa;
    uint64_t*kernel_root=(uint64_t*)(uintptr_t)mmu_kernel_root_address();
    for(unsigned i=0;i<512;i++)root[i]=kernel_root[i];
    zero_page(l2_pa);zero_page(l3_pa);zero_page(data_pa);
    root[64]=(l2_pa&~(PAGE_SIZE-1u))|DESC_VALID_TABLE;
    l2[0]=(l3_pa&~(PAGE_SIZE-1u))|DESC_VALID_TABLE;
    l3[0]=(data_pa&~(PAGE_SIZE-1u))|DESC_AF|DESC_SH_INNER|DESC_ATTR_NORMAL|DESC_AP_EL0_RW|DESC_PXN|DESC_UXN|DESC_VALID_TABLE;
    mmu_switch_root(root_pa);
    volatile uint64_t*user=(volatile uint64_t*)(uintptr_t)USER_TEST_VA;
    user[0]=0xded05a11ull;
    user[511]=0x41524d36345f564dull;
    bool valid=user[0]==0xded05a11ull&&user[511]==0x41524d36345f564dull;
    mmu_switch_root(mmu_kernel_root_address());
    bool freed_data=memory_free_page(data_pa);
    bool freed_l3=memory_free_page(l3_pa);
    bool freed_l2=memory_free_page(l2_pa);
    bool freed_root=memory_free_page(root_pa);
    return valid&&freed_data&&freed_l3&&freed_l2&&freed_root;
}
