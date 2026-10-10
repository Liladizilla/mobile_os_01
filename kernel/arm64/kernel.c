#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#define UART0_BASE 0x09000000UL
#define UART_DR (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_FR_RXFE (1u << 4)
#define UART_FR_TXFF (1u << 5)
#define INPUT_CAPACITY 96
static void uart_putc(char c){if(c=='\n'){while(UART_FR&UART_FR_TXFF){}UART_DR='\r';}while(UART_FR&UART_FR_TXFF){}UART_DR=(uint32_t)c;}
void uart_write_external(const char*s){while(*s)uart_putc(*s++);}
void uart_hex_external(uint64_t v){static const char d[]="0123456789abcdef";uart_write_external("0x");for(int i=60;i>=0;i-=4)uart_putc(d[(v>>i)&15u]);}
static char uart_getc(void){while(UART_FR&UART_FR_RXFE){}return(char)(UART_DR&255u);}
static bool equal(const char*a,const char*b){while(*a&&*b&&*a==*b){a++;b++;}return !*a&&!*b;}
static void help(void){uart_write_external("\nDedus kernel commands:\n  help      Show commands\n  uname     Kernel and architecture\n  features  Implemented kernel foundations\n  meminfo   Device-tree RAM and allocator report\n  memtest   Allocate/free two pages and verify accounting\n  clear     Clear terminal\n  about     Explain this target\n  fault     Trigger exception-handler test\n");}
static void command(const char*s){
if(equal(s,"help"))help();
else if(equal(s,"uname"))uart_write_external("\nDedus Mobile OS kernel 0.3.0 (aarch64 / ARMv8-A)\n");
else if(equal(s,"features")){uart_write_external("\nImplemented: AArch64 entry, EL2-to-EL1 normalization, BSS init, EL1 exception vectors,");uart_write_external("\nPL011 UART, FDT RAM discovery, 4 KiB physical page allocator, shell and panic diagnostics.\n");uart_write_external("Not implemented: MMU, scheduler, userspace, storage, display/touch, GPU, modem, Wi-Fi, audio, camera or phone-specific power management.\n");}
else if(equal(s,"meminfo"))memory_print_info();
else if(equal(s,"memtest")){uart_write_external("\nPhysical page allocator self-test: ");uart_write_external(memory_self_test()?"PASS. Two distinct 4 KiB pages allocated and released; free-page count restored.\n":"FAIL. Check FDT RAM data and allocator initialization.\n");}
else if(equal(s,"clear"))uart_write_external("\033[2J\033[H");
else if(equal(s,"about")){uart_write_external("\nDedus is a freestanding ARM64 kernel targeting QEMU virt.");uart_write_external("\nIt is not yet installable on a retail phone. A phone port needs SoC-specific boot flow, device tree, drivers and verified recovery.\n");}
else if(equal(s,"fault")){uart_write_external("\nTriggering intentional BRK exception; panic diagnostics should follow.\n");__asm__ volatile("brk #0");}
else if(*s){uart_write_external("\nUnknown command: ");uart_write_external(s);uart_write_external("\nType 'help' for commands.\n");}
}
__attribute__((noreturn)) void exception_handler(uint64_t vector,uint64_t esr,uint64_t elr,uint64_t far){uart_write_external("\n\n*** DEDUS KERNEL EXCEPTION ***\nVector slot: ");uart_hex_external(vector);uart_write_external("\nESR_EL1: ");uart_hex_external(esr);uart_write_external("\nELR_EL1: ");uart_hex_external(elr);uart_write_external("\nFAR_EL1: ");uart_hex_external(far);uart_write_external("\nKernel halted to avoid unsafe exception return.\n");for(;;)__asm__ volatile("wfe");}
void kernel_main(uint64_t dtb){char input[INPUT_CAPACITY];size_t used=0;uart_write_external("\n\n  DEDUS MOBILE OS | ARM64 KERNEL 0.3.0\n  -----------------------------------\n");uart_write_external("  Target: QEMU virt / ARMv8-A.\n  EL1 exception vectors installed.\n");uart_write_external(memory_is_ready()?"  Device-tree RAM map and 4 KiB page allocator ready.\n":"  RAM allocator unavailable; FDT absent or unusable.\n");uart_write_external("  Serial console ready. Type 'help'.\n\ndedus> ");for(;;){char ch=uart_getc();if(ch=='\r'||ch=='\n'){input[used]=0;command(input);used=0;uart_write_external("\ndedus> ");}else if(ch=='\b'||ch==0x7f){if(used){--used;uart_write_external("\b \b");}}else if(ch>=0x20&&ch<=0x7e){if(used+1<INPUT_CAPACITY){input[used++]=ch;uart_putc(ch);}else{uart_write_external("\nInput too long; command discarded.\ndedus> ");used=0;}}}}
