/*
 * Adizilla ARM64 bare-metal kernel milestone.
 * Runs on the QEMU "virt" ARMv8 board using its PL011 UART.
 * This is kernel code, not the browser-based OS mock-up.
 * SPDX-License-Identifier: MIT
 */
#include <stddef.h>
#include <stdint.h>

#define UART0_BASE 0x09000000UL
#define UART_DR (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_FR_RXFE (1u << 4)
#define UART_FR_TXFF (1u << 5)
#define INPUT_CAPACITY 96

static void uart_putc(char c) {
    if (c == '\n') {
        while (UART_FR & UART_FR_TXFF) { }
        UART_DR = '\r';
    }
    while (UART_FR & UART_FR_TXFF) { }
    UART_DR = (uint32_t)c;
}

static void uart_write(const char *text) {
    while (*text) uart_putc(*text++);
}

static void uart_write_hex(uint64_t value) {
    static const char digits[] = "0123456789abcdef";
    uart_write("0x");
    for (int shift = 60; shift >= 0; shift -= 4) {
        uart_putc(digits[(value >> shift) & 0xfu]);
    }
}

static char uart_getc(void) {
    while (UART_FR & UART_FR_RXFE) { }
    return (char)(UART_DR & 0xffu);
}

static int string_equal(const char *left, const char *right) {
    while (*left && *right && *left == *right) {
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

static void print_help(void) {
    uart_write("\nCommands available in this kernel milestone:\n");
    uart_write("  help      Show commands\n");
    uart_write("  uname     Show kernel/architecture\n");
    uart_write("  features  Show implemented hardware-level features\n");
    uart_write("  clear     Clear the serial terminal\n");
    uart_write("  about     Explain this build target\n");
    uart_write("  fault     Trigger a deliberate exception-handler test\n");
}

static void run_command(const char *command) {
    if (string_equal(command, "help")) {
        print_help();
    } else if (string_equal(command, "uname")) {
        uart_write("\nAdizilla Mobile OS kernel 0.3.0 (aarch64 / ARMv8-A)\n");
    } else if (string_equal(command, "features")) {
        uart_write("\nImplemented: AArch64 entry, EL2-to-EL1 normalization, BSS init,");
        uart_write("\nEL1 exception vectors, PL011 UART input/output, serial shell,");
        uart_write("\nfreestanding C build and linker-defined image.");
        uart_write("\nNot implemented yet: MMU, scheduler, userspace, timer/GIC, touchscreen,");
        uart_write("\nGPU, storage, modem, Wi-Fi, audio, camera, power management.\n");
    } else if (string_equal(command, "clear")) {
        uart_write("\033[2J\033[H");
    } else if (string_equal(command, "about")) {
        uart_write("\nThis is a freestanding ARM64 kernel running on QEMU virt.");
        uart_write("\nIt is not yet installable on a retail phone. Phone support needs");
        uart_write("\nSoC-specific boot flow, device tree, drivers, and verified images.\n");
    } else if (string_equal(command, "fault")) {
        uart_write("\nTriggering intentional BRK exception; panic output should follow.\n");
        __asm__ volatile("brk #0");
    } else if (command[0] != '\0') {
        uart_write("\nUnknown command: ");
        uart_write(command);
        uart_write("\nType 'help' for commands.\n");
    }
}

__attribute__((noreturn))
void exception_handler(uint64_t vector, uint64_t esr, uint64_t elr, uint64_t far) {
    uart_write("\n\n*** ADIZILLA KERNEL EXCEPTION ***\n");
    uart_write("Vector slot: ");
    uart_write_hex(vector);
    uart_write("\nESR_EL1:     ");
    uart_write_hex(esr);
    uart_write("\nELR_EL1:     ");
    uart_write_hex(elr);
    uart_write("\nFAR_EL1:     ");
    uart_write_hex(far);
    uart_write("\nKernel halted to avoid unsafe exception return.\n");

    for (;;) {
        __asm__ volatile("wfe");
    }
}

void kernel_main(void) {
    char input[INPUT_CAPACITY];
    size_t used = 0;

    uart_write("\n\n");
    uart_write("  ADIZILLA MOBILE OS | ARM64 KERNEL 0.3.0\n");
    uart_write("  --------------------------------------\n");
    uart_write("  Booted on QEMU virt / ARMv8-A.\n");
    uart_write("  EL1 exception vectors installed.\n");
    uart_write("  Serial console is live. Type 'help'.\n\n");
    uart_write("adz> ");

    for (;;) {
        char c = uart_getc();

        if (c == '\r' || c == '\n') {
            input[used] = '\0';
            run_command(input);
            used = 0;
            uart_write("\nadz> ");
        } else if (c == '\b' || c == 0x7f) {
            if (used > 0) {
                --used;
                uart_write("\b \b");
            }
        } else if (c >= 0x20 && c <= 0x7e) {
            if (used + 1 < INPUT_CAPACITY) {
                input[used++] = c;
                uart_putc(c);
            } else {
                uart_write("\nInput too long; command discarded.\nadz> ");
                used = 0;
            }
        }
    }
}
