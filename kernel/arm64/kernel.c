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
static uint64_t l1_table[512] __attribute__((aligned(4096)));
static uint64_t l2_low[512] __attribute__((aligned(4096)));
static uint64_t l2_ram[512] __attribute__((aligned(4096)));
static uint64_t l3_kernel[4][512] __attribute__((aligned(4096)));
extern char __kernel_start[], __kernel_end[], __text_start[], __text_end[];
extern char __vectors_start[], __vectors_end[], __rodata_start[], __rodata_end[];
extern char __data_start[], __data_end[], __bss_start[], __bss_end[];
static bool mmu_ready;
static uint64_t timer_reload;
static bool timer_ready;
static void uart_putc(char c){if(c=='\n'){while(UART_FR&UART_FR_TXFF){}UART_DR='\r';}while(UART_FR&UART_FR_TXFF){}UART_DR=(uint32_t)c;}
void uart_write_external(const char*s){while(*s)uart_putc(*s++);}
void uart_hex_external(uint64_t v){static const char d[]="0123456789abcdef";uart_write_external("0x");for(int i=60;i>=0;i-=4)uart_putc(d[(v>>i)&15u]);}
static char uart_getc(void){while(UART_FR&UART_FR_RXFE){}return(char)(UART_DR&255u);}
static bool equal(const char*a,const char*b){while(*a&&*b&&*a==*b){a++;b++;}return !*a&&!*b;}
static inline void dsb_sy(void){__asm__ volatile("dsb sy" ::: "memory");}
static inline void isb(void){__asm__ volatile("isb" ::: "memory");}
static bool page_intersects(uint64_t page, const char *start, const char *end){
    uint64_t a=(uint64_t)(uintptr_t)start,b=(uint64_t)(uintptr_t)end;
    return b>a && page<b && page+4096u>a;
}
static uint64_t normal_block(uint64_t physical){
    return physical|(1ull<<10)|(3ull<<8)|(1ull<<2)|(1ull<<53)|(1ull<<54)|1ull;
}
static uint64_t normal_page(uint64_t physical, bool executable, bool read_only){
    uint64_t d=physical|(1ull<<10)|(3ull<<8)|(1ull<<2)|3ull;
    if(read_only)d|=(1ull<<7);
    if(!executable)d|=(1ull<<53)|(1ull<<54);
    return d;
}
static void mmu_init(void){
    for(unsigned i=0;i<512;i++){
        l1_table[i]=0;
        l2_low[i]=((uint64_t)i<<21)|(1ull<<10)|(1ull<<53)|(1ull<<54)|1ull;
        l2_ram[i]=normal_block(0x40000000ull+((uint64_t)i<<21));
    }
    for(unsigned t=0;t<4;t++)for(unsigned p=0;p<512;p++)l3_kernel[t][p]=0;
    l1_table[0]=((uint64_t)(uintptr_t)l2_low&~0xfffull)|3ull;
    l1_table[1]=((uint64_t)(uintptr_t)l2_ram&~0xfffull)|3ull;
    l1_table[2]=normal_block(0x80000000ull);
    l1_table[3]=normal_block(0xc0000000ull);
    uint64_t kstart=(uint64_t)(uintptr_t)__kernel_start;
    uint64_t kend=(uint64_t)(uintptr_t)__kernel_end;
    if(kend<=kstart || kend-kstart>8u*1024u*1024u)return;
    unsigned first=(unsigned)((kstart-0x40000000ull)>>21);
    unsigned last=(unsigned)((kend-1u-0x40000000ull)>>21);
    if(last>=4u)return;
    for(unsigned block=first;block<=last;block++){
        uint64_t base=0x40000000ull+((uint64_t)block<<21);
        for(unsigned p=0;p<512;p++){
            uint64_t page=base+((uint64_t)p<<12);
            bool code=page_intersects(page,__text_start,__text_end)||page_intersects(page,__vectors_start,__vectors_end);
            bool rodata=page_intersects(page,__rodata_start,__rodata_end);
            bool ro=code||rodata;
            l3_kernel[block][p]=normal_page(page,code,ro);
        }
        l2_ram[block]=((uint64_t)(uintptr_t)l3_kernel[block]&~0xfffull)|3ull;
    }
    uint64_t mair=0xff00ull;
    uint64_t tcr=25ull|(1ull<<8)|(1ull<<10)|(3ull<<12);
    uint64_t ttbr=(uint64_t)(uintptr_t)l1_table;
    __asm__ volatile("msr mair_el1, %0"::"r"(mair));
    __asm__ volatile("msr tcr_el1, %0"::"r"(tcr));
    __asm__ volatile("msr ttbr0_el1, %0"::"r"(ttbr));
    dsb_sy(); isb();
    __asm__ volatile("tlbi vmalle1" ::: "memory");
    dsb_sy(); isb();
    uint64_t sctlr;
    __asm__ volatile("mrs %0, sctlr_el1":"=r"(sctlr));
    sctlr|=(1ull<<0)|(1ull<<2)|(1ull<<12);
    __asm__ volatile("msr sctlr_el1, %0"::"r"(sctlr):"memory");
    isb();
    mmu_ready=true;
}
static void timer_arm(void){__asm__ volatile("msr cntv_tval_el0, %0"::"r"(timer_reload));__asm__ volatile("msr cntv_ctl_el0, %0"::"r"((uint64_t)1));isb();}
static void timer_init(void){
    uint64_t frequency;
    __asm__ volatile("mrs %0, cntfrq_el0":"=r"(frequency));
    if(frequency<100u)return;
    timer_reload=frequency/10u;
    /* QEMU virt with GICv2: use Group 0 for the EL1 virtual timer in this boot security state. */
    GICD_CTLR=0;
    GICD_IGROUPR0&=~(1u<<TIMER_IRQ_ID);
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
static void help(void){uart_write_external("\nDedus kernel commands:\n  help      Show commands\n  uname     Kernel and architecture\n  features  Implemented kernel foundations\n  meminfo   Device-tree RAM and allocator report\n  memtest   Allocate/free two pages and verify accounting\n  ticks     Show generic timer interrupt count\n  timer     Timer/GIC initialization status\n  mmu       MMU and cache status\n  clear     Clear terminal\n  about     Explain this target\n  vectors   Verify exception vector base/alignment\n  fault     Trigger exception-handler test\n  protectfault Attempt write to read-only kernel text (expected fault)\n");}
static void command(const char*s){
if(equal(s,"help"))help();
else if(equal(s,"uname"))uart_write_external("\nDedus Mobile OS kernel 0.4.0 (aarch64 / ARMv8-A)\n");
else if(equal(s,"features")){uart_write_external("\nImplemented: AArch64 entry, EL2-to-EL1 normalization, BSS init, aligned EL1 exception vectors, MMU identity mapping and cache enablement,");uart_write_external("\nPL011 UART, FDT RAM discovery, 4 KiB physical page allocator, GICv2 virtual timer IRQ, shell and panic diagnostics.\n");uart_write_external("Not implemented: MMU, scheduler, userspace, storage, display/touch, GPU, modem, Wi-Fi, audio, camera or phone-specific power management.\n");}
else if(equal(s,"meminfo"))memory_print_info();
else if(equal(s,"memtest")){uart_write_external("\nPhysical page allocator self-test: ");uart_write_external(memory_self_test()?"PASS. Two distinct 4 KiB pages allocated and released; free-page count restored.\n":"FAIL. Check FDT RAM data and allocator initialization.\n");}
else if(equal(s,"ticks")){uart_write_external("\nTimer IRQ count: ");uart_hex_external(timer_ticks);uart_write_external(timer_ready?" (timer enabled)\n":" (timer not initialized)\n");}
else if(equal(s,"mmu")){uint64_t sctlr;__asm__ volatile("mrs %0, sctlr_el1":"=r"(sctlr));uart_write_external("\nMMU status: ");uart_write_external((sctlr&1u)?"enabled":"disabled");uart_write_external("\nSCTLR_EL1: ");uart_hex_external(sctlr);uart_write_external(mmu_ready?"\nIdentity map: 0-1 GiB device memory; 1-4 GiB normal memory; kernel text/vector RO+X, rodata RO+XN, data RW+XN\n":"\nIdentity map not initialized\n");}
else if(equal(s,"timer")){uint64_t ctl,tval,daif,freq;__asm__ volatile("mrs %0, cntv_ctl_el0":"=r"(ctl));__asm__ volatile("mrs %0, cntv_tval_el0":"=r"(tval));__asm__ volatile("mrs %0, daif":"=r"(daif));__asm__ volatile("mrs %0, cntfrq_el0":"=r"(freq));uart_write_external("\nGeneric virtual timer / GICv2 status: ");uart_write_external(timer_ready?"initialized; expected 10 IRQs/second\n":"NOT INITIALIZED\n");uart_write_external("CNTFRQ_EL0: ");uart_hex_external(freq);uart_write_external("\nCNTV_CTL_EL0: ");uart_hex_external(ctl);uart_write_external("\nCNTV_TVAL_EL0: ");uart_hex_external(tval);uart_write_external("\nDAIF: ");uart_hex_external(daif);uart_write_external("\nGICD_CTLR: ");uart_hex_external(GICD_CTLR);uart_write_external("\nGICC_CTLR: ");uart_hex_external(GICC_CTLR);uart_write_external("\nGICD_IGROUPR0: ");uart_hex_external(GICD_IGROUPR0);uart_write_external("\nGICD_ISENABLER0: ");uart_hex_external(GICD_ISENABLER0);uart_write_external("\n");}
else if(equal(s,"clear"))uart_write_external("\033[2J\033[H");
else if(equal(s,"about")){uart_write_external("\nDedus is a freestanding ARM64 kernel targeting QEMU virt.");uart_write_external("\nIt is not yet installable on a retail phone. A phone port needs SoC-specific boot flow, device tree, drivers and verified recovery.\n");}
else if(equal(s,"vectors")){uint64_t base,current;__asm__ volatile("mrs %0, vbar_el1":"=r"(base));__asm__ volatile("mrs %0, CurrentEL":"=r"(current));uart_write_external("\nCurrentEL: ");uart_hex_external(current);uart_write_external("\nVBAR_EL1: ");uart_hex_external(base);uart_write_external("\nVector table: ");uart_hex_external((uint64_t)(uintptr_t)exception_vectors);uart_write_external("\nVector base check: ");uart_write_external((base==(uint64_t)(uintptr_t)exception_vectors&&!(base&0x7ffu))?"PASS\n":"FAIL\n");}
else if(equal(s,"protectfault")){uart_write_external("\nAttempting to write to read-only kernel text; a permission fault is expected.\n");volatile uint32_t *p=(volatile uint32_t *)(uintptr_t)__text_start;uint32_t value=*p;*p=value;uart_write_external("ERROR: kernel text write unexpectedly succeeded.\n");}
else if(equal(s,"fault")){uart_write_external("\nTriggering intentional SVC exception; panic diagnostics should follow.\n");__asm__ volatile("svc #0");}
else if(*s){uart_write_external("\nUnknown command: ");uart_write_external(s);uart_write_external("\nType 'help' for commands.\n");}
}
__attribute__((noreturn)) void exception_handler(uint64_t vector,uint64_t esr,uint64_t elr,uint64_t far){uart_write_external("\n\n*** DEDUS KERNEL EXCEPTION ***\nVector slot: ");uart_hex_external(vector);uart_write_external("\nESR_EL1: ");uart_hex_external(esr);uart_write_external("\nELR_EL1: ");uart_hex_external(elr);uart_write_external("\nFAR_EL1: ");uart_hex_external(far);uart_write_external("\nKernel halted to avoid unsafe exception return.\n");for(;;)__asm__ volatile("wfe");}
void kernel_main(uint64_t dtb){char input[INPUT_CAPACITY];size_t used=0;uart_write_external("\n\n  DEDUS MOBILE OS | ARM64 KERNEL 0.4.0\n  -----------------------------------\n");uart_write_external("  Target: QEMU virt / ARMv8-A / GICv2.\n  EL1 exception vectors installed.\n");uart_write_external("  FDT argument: ");uart_hex_external(dtb);uart_write_external("\n");memory_init(dtb);uart_write_external(memory_is_ready()?"  Device-tree RAM map and 4 KiB page allocator ready.\n":"  RAM allocator unavailable; FDT absent or unusable.\n");mmu_init();uart_write_external(mmu_ready?"  EL1 MMU and caches enabled with identity mappings.\n":"  MMU initialization failed.\n");timer_init();uart_write_external(timer_ready?"  Generic virtual timer IRQ and GICv2 initialized.\n":"  Generic virtual timer initialization failed.\n");uart_write_external("  Serial console ready. Type 'help'.\n\ndedus> ");for(;;){char ch=uart_getc();if(ch=='\r'||ch=='\n'){input[used]=0;command(input);used=0;uart_write_external("\ndedus> ");}else if(ch=='\b'||ch==0x7f){if(used){--used;uart_write_external("\b \b");}}else if(ch>=0x20&&ch<=0x7e){if(used+1<INPUT_CAPACITY){input[used++]=ch;uart_putc(ch);}else{uart_write_external("\nInput too long; command discarded.\ndedus> ");used=0;}}}}
