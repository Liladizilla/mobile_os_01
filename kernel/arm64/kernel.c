#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#define UART0_BASE 0x09000000UL
#define UART_DR (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_FR_RXFE (1u << 4)
#define UART_FR_TXFF (1u << 5)
#define GICD_BASE 0x08000000UL
#define GICC_BASE 0x08010000UL
#define GICD_CTLR (*(volatile uint32_t *)(GICD_BASE + 0x000))
#define GICD_IGROUPR0 (*(volatile uint32_t *)(GICD_BASE + 0x080))
#define GICD_ISENABLER0 (*(volatile uint32_t *)(GICD_BASE + 0x100))
#define GICD_IPRIORITYR27 (*(volatile uint8_t *)(GICD_BASE + 0x400 + 27))
#define GICC_CTLR (*(volatile uint32_t *)(GICC_BASE + 0x000))
#define GICC_PMR (*(volatile uint32_t *)(GICC_BASE + 0x004))
#define GICC_IAR (*(volatile uint32_t *)(GICC_BASE + 0x00c))
#define GICC_EOIR (*(volatile uint32_t *)(GICC_BASE + 0x010))
#define TIMER_IRQ_ID 27u
#define INPUT_CAPACITY 96
extern uint8_t exception_vectors[];
static volatile uint64_t timer_ticks;
static uint64_t timer_reload;
static bool timer_ready;
static void uart_putc(char c){if(c=='\n'){while(UART_FR&UART_FR_TXFF){}UART_DR='\r';}while(UART_FR&UART_FR_TXFF){}UART_DR=(uint32_t)c;}
void uart_write_external(const char*s){while(*s)uart_putc(*s++);}
void uart_hex_external(uint64_t v){static const char d[]="0123456789abcdef";uart_write_external("0x");for(int i=60;i>=0;i-=4)uart_putc(d[(v>>i)&15u]);}
static char uart_getc(void){while(UART_FR&UART_FR_RXFE){}return(char)(UART_DR&255u);}
static bool equal(const char*a,const char*b){while(*a&&*b&&*a==*b){a++;b++;}return !*a&&!*b;}
static inline void dsb_sy(void){__asm__ volatile("dsb sy" ::: "memory");}
static inline void isb(void){__asm__ volatile("isb" ::: "memory");}
static void timer_arm(void){__asm__ volatile("msr cntv_tval_el0, %0"::"r"(timer_reload));__asm__ volatile("msr cntv_ctl_el0, %0"::"r"((uint64_t)1));isb();}
static void timer_init(void){
    uint64_t frequency;
    __asm__ volatile("mrs %0, cntfrq_el0":"=r"(frequency));
    if(frequency<100u)return;
    timer_reload=frequency/10u;
    /* QEMU virt with GICv2: route the EL1 virtual timer as Group 1. */
    GICD_CTLR=0;
    GICD_IGROUPR0|=(1u<<TIMER_IRQ_ID);
    GICD_IPRIORITYR27=0x80;
    GICD_ISENABLER0=(1u<<TIMER_IRQ_ID);
    GICC_PMR=0xff;
    GICC_CTLR=1;
    GICD_CTLR=1;
    dsb_sy();
    timer_arm();
    timer_ready=true;
    __asm__ volatile("msr daifclr, #2" ::: "memory");
    isb();
}
void timer_irq_handler(void){
    uint32_t iar=GICC_IAR;
    uint32_t id=iar&0x3ffu;
    if(id==TIMER_IRQ_ID){timer_ticks++;timer_arm();}
    if(id<1020u){dsb_sy();GICC_EOIR=iar;dsb_sy();}
}
static void help(void){uart_write_external("\nDedus kernel commands:\n  help      Show commands\n  uname     Kernel and architecture\n  features  Implemented kernel foundations\n  meminfo   Device-tree RAM and allocator report\n  memtest   Allocate/free two pages and verify accounting\n  ticks     Show generic timer interrupt count\n  timer     Timer/GIC initialization status\n  clear     Clear terminal\n  about     Explain this target\n  vectors   Verify exception vector base/alignment\n  fault     Trigger exception-handler test\n");}
static void command(const char*s){
if(equal(s,"help"))help();
else if(equal(s,"uname"))uart_write_external("\nDedus Mobile OS kernel 0.4.0 (aarch64 / ARMv8-A)\n");
else if(equal(s,"features")){uart_write_external("\nImplemented: AArch64 entry, EL2-to-EL1 normalization, BSS init, aligned EL1 exception vectors,");uart_write_external("\nPL011 UART, FDT RAM discovery, 4 KiB physical page allocator, GICv2 virtual timer IRQ, shell and panic diagnostics.\n");uart_write_external("Not implemented: MMU, scheduler, userspace, storage, display/touch, GPU, modem, Wi-Fi, audio, camera or phone-specific power management.\n");}
else if(equal(s,"meminfo"))memory_print_info();
else if(equal(s,"memtest")){uart_write_external("\nPhysical page allocator self-test: ");uart_write_external(memory_self_test()?"PASS. Two distinct 4 KiB pages allocated and released; free-page count restored.\n":"FAIL. Check FDT RAM data and allocator initialization.\n");}
else if(equal(s,"ticks")){uart_write_external("\nTimer IRQ count: ");uart_hex_external(timer_ticks);uart_write_external(timer_ready?" (timer enabled)\n":" (timer not initialized)\n");}
else if(equal(s,"timer")){uart_write_external("\nGeneric virtual timer / GICv2 status: ");uart_write_external(timer_ready?"initialized; expected 10 IRQs/second\n":"NOT INITIALIZED\n");}
else if(equal(s,"clear"))uart_write_external("\033[2J\033[H");
else if(equal(s,"about")){uart_write_external("\nDedus is a freestanding ARM64 kernel targeting QEMU virt.");uart_write_external("\nIt is not yet installable on a retail phone. A phone port needs SoC-specific boot flow, device tree, drivers and verified recovery.\n");}
else if(equal(s,"vectors")){uint64_t base,current;__asm__ volatile("mrs %0, vbar_el1":"=r"(base));__asm__ volatile("mrs %0, CurrentEL":"=r"(current));uart_write_external("\nCurrentEL: ");uart_hex_external(current);uart_write_external("\nVBAR_EL1: ");uart_hex_external(base);uart_write_external("\nVector table: ");uart_hex_external((uint64_t)(uintptr_t)exception_vectors);uart_write_external("\nVector base check: ");uart_write_external((base==(uint64_t)(uintptr_t)exception_vectors&&!(base&0x7ffu))?"PASS\n":"FAIL\n");}
else if(equal(s,"fault")){uart_write_external("\nTriggering intentional SVC exception; panic diagnostics should follow.\n");__asm__ volatile("svc #0");}
else if(*s){uart_write_external("\nUnknown command: ");uart_write_external(s);uart_write_external("\nType 'help' for commands.\n");}
}
__attribute__((noreturn)) void exception_handler(uint64_t vector,uint64_t esr,uint64_t elr,uint64_t far){uart_write_external("\n\n*** DEDUS KERNEL EXCEPTION ***\nVector slot: ");uart_hex_external(vector);uart_write_external("\nESR_EL1: ");uart_hex_external(esr);uart_write_external("\nELR_EL1: ");uart_hex_external(elr);uart_write_external("\nFAR_EL1: ");uart_hex_external(far);uart_write_external("\nKernel halted to avoid unsafe exception return.\n");for(;;)__asm__ volatile("wfe");}
void kernel_main(uint64_t dtb){char input[INPUT_CAPACITY];size_t used=0;uart_write_external("\n\n  DEDUS MOBILE OS | ARM64 KERNEL 0.4.0\n  -----------------------------------\n");uart_write_external("  Target: QEMU virt / ARMv8-A / GICv2.\n  EL1 exception vectors installed.\n");uart_write_external("  FDT argument: ");uart_hex_external(dtb);uart_write_external("\n");memory_init(dtb);uart_write_external(memory_is_ready()?"  Device-tree RAM map and 4 KiB page allocator ready.\n":"  RAM allocator unavailable; FDT absent or unusable.\n");timer_init();uart_write_external(timer_ready?"  Generic virtual timer IRQ and GICv2 initialized.\n":"  Generic virtual timer initialization failed.\n");uart_write_external("  Serial console ready. Type 'help'.\n\ndedus> ");for(;;){char ch=uart_getc();if(ch=='\r'||ch=='\n'){input[used]=0;command(input);used=0;uart_write_external("\ndedus> ");}else if(ch=='\b'||ch==0x7f){if(used){--used;uart_write_external("\b \b");}}else if(ch>=0x20&&ch<=0x7e){if(used+1<INPUT_CAPACITY){input[used++]=ch;uart_putc(ch);}else{uart_write_external("\nInput too long; command discarded.\ndedus> ");used=0;}}}}
