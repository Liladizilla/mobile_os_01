# Dedus Mobile OS: real-kernel plan

## Truth about the current repository

The existing browser UI is a useful product prototype, not a phone operating system. The older C files use hosted-library calls such as `stdio`, `stdlib`, and `calloc`; the networking implementation simulates interfaces; and the old ISO scripts write a placeholder kernel image. Those pieces must not be represented as a functioning phone kernel.

The new `kernel/arm64` target is a genuine low-level milestone: freestanding C plus AArch64 assembly, a linker script, direct PL011 UART hardware access, an interactive serial shell, EL2-to-EL1 normalization, BSS initialization, and an EL1 exception vector table with diagnostic panic output, flattened-device-tree RAM discovery, and a bitmap-backed 4 KiB physical page allocator with an allocation/free self-test. It is intended to boot under QEMU's ARM `virt` machine. It does **not** yet boot on a retail phone.

## Architecture

- **Kernel language:** C for kernel subsystems, AArch64 assembly for entry and exception-sensitive code.
- **Initial execution target:** QEMU `virt`, ARMv8-A, PL011 UART.
- **Build:** `aarch64-none-elf-gcc` and GNU binutils.
- **Emulation:** `qemu-system-aarch64`.
- **Phone target:** not selected yet. A production phone port needs an exact model, SoC, boot chain, device tree, and driver support. The user's Samsung Galaxy M04 is a MediaTek device; support cannot be inferred from a generic ARM64 build.

## Build and run

Install an AArch64 bare-metal GCC/binutils toolchain and QEMU system emulator, then run:

```sh
make arm64
make run-arm64
```

Or directly:

```sh
make -C kernel/arm64
make -C kernel/arm64 run
```

QEMU opens a serial console. Type `help`, `uname`, `features`, `clear`, or `about`. The `fault` command deliberately executes a BRK instruction to test exception diagnostics; the expected result is a printed syndrome/register report followed by a halt.

## Roadmap

1. **Bootable ARM64 kernel:** startup assembly, linker layout, UART input/output, serial shell, BSS initialization and exception diagnostics.
2. **Memory foundations:** parse the QEMU device tree, discover RAM, reserve kernel/FDT pages, and test a 4 KiB physical page allocator.
3. **Interrupt foundations:** configure the generic timer and GICv2/v3, then enable controlled interrupts.
4. **Virtual memory and isolation:** add MMU mappings only after memory ownership is tested.
5. **Kernel services:** preemptive scheduling, process/thread model, syscalls, virtual filesystem and block storage driver.
6. **Mobile platform:** device-tree parsing for a chosen device, display pipeline, input/touch, battery/charging, suspend/resume, audio, Wi-Fi, modem and sensors.
7. **User environment:** secure init/service manager, permissions, app packaging/runtime, notifications, settings, launcher, file manager, accessibility, theme and visual effects.
8. **Retain prototype features as services/apps:** terminal, notes, calculator, calendar, task manager, weather/news/crypto clients, AI assistant, telemetry dashboards, multi-user sessions and audit tools. Network-backed/API features belong in user-space apps, not the kernel.
9. **Security:** verified boot integration, memory/process isolation, capability/permission model, secure update path, encryption using vetted libraries, sandboxing and audit logging. Never claim a firewall, intrusion detection, 2FA, encryption, or malware scanner is implemented until it is tested and real.
10. **Device port and validation:** choose a supported device and boot chain; document recovery/rollback; test on emulation first and never flash an unverified image to a daily-use phone.

## Non-goals for this milestone

- No TypeScript/JavaScript in the kernel.
- No claim that the current ISO installs on a phone.
- No fake CPU/RAM/network metrics or simulated devices presented as hardware telemetry.
- No overwriting Android partitions or flashing instructions until a device-specific port is proven and recoverable.


## UI/UX principles for Dedus

The existing `frontend/` is a clickable product prototype, not a native interface running on the kernel. Keep it as a design sandbox until graphics, input, scheduling and user-space services exist.

- **Design language:** solid morphism: solid surfaces, crisp shadow outlines, restrained depth and consistent spacing/type. Blur is optional, never required for legibility.
- **Mobile-first ergonomics:** thumb-reachable primary actions, large touch targets, edge-safe layouts, one-handed navigation, and predictable Back/Home/Recents behavior.
- **Information hierarchy:** home, app switcher, notifications/quick settings, lock screen, settings and file manager each need one clear primary task. Avoid dashboard clutter and fake system metrics.
- **Performance tiers:** solid-color/software fallback first; blur, live wallpaper and complex shadows only when the compositor can sustain them. Reduce effects before responsiveness.
- **Accessibility:** scalable text, high contrast, visible focus, screen-reader semantics, reduced motion and non-gesture alternatives.
- **Security UX:** no default root/admin passwords in demos. Permissions must explain what they grant; privacy indicators must reflect real device services.
- **Honest system state:** CPU/memory/storage/network/battery values must come from real telemetry. Simulated prototype values must be labelled.


## Current memory milestone

The kernel receives the flattened device tree pointer from the boot protocol, parses the FDT structure to discover the first RAM region, and reserves the kernel image and FDT pages. A bitmap tracks up to 1 GiB of 4 KiB pages. The `meminfo` command reports the discovered region and pool; `memtest` allocates two distinct pages, releases them, and verifies that free-page accounting returns to its starting value. This is an initial QEMU-targeted allocator, not yet a general multi-region physical memory manager.

## UI/UX direction

See [UX_PRINCIPLES.md](UX_PRINCIPLES.md). Dedus uses solid morphism: solid surfaces, crisp shadow outlines, restrained depth and optional blur. The browser prototype is not a native GUI. Touch/display drivers, a compositor, accessible input, and user-space services must exist before the design can become the actual phone interface.


## Timer interrupt milestone (QEMU only)

The ARM64 target now configures the QEMU `virt` board's GICv2 and EL1 virtual generic timer. A dedicated IRQ vector saves and restores general-purpose registers, acknowledges the interrupt, increments a kernel tick counter, rearms the timer, and completes the GIC interrupt. The `timer` command exposes timer/GIC registers for bring-up diagnostics; `ticks` reports the delivered interrupt count. CI requires the count to advance before it uploads the ELF and raw kernel artifacts.

This is not yet a scheduler clock or a phone-portable interrupt controller abstraction. Before relying on it for preemption, add exception-frame tests, timer drift/overflow handling, interrupt-controller abstraction, and tests for repeated IRQ entry/exit. The current target is specifically QEMU `virt` with GICv2.


## Bootstrap MMU milestone (QEMU only)

The kernel now builds a 4 KiB-granule TTBR0 translation table and enables the EL1 MMU, data cache and instruction cache. The first GiB is identity-mapped with Device attributes for the QEMU peripheral window; the 1-4 GiB range is identity-mapped as normal inner-shareable memory, covering the QEMU RAM and kernel image. The `mmu` command checks SCTLR_EL1 and reports the mapping policy. CI runs this command after enabling translation and continues to test the serial shell, page allocator, GICv2 timer and exception path.

The QEMU RAM window is now split into 2 MiB blocks, with a 4 KiB page table for the kernel image. Kernel text and exception vectors are read-only and executable; rodata is read-only and execute-never; data/BSS and stack are read-write and execute-never. Other RAM blocks remain broad identity mappings, and the low peripheral window uses Device attributes. A second QEMU boot runs `protectfault`, attempts to write the read-only text page, and requires a data-abort diagnostic at the expected kernel address. CI must pass both the regular shell/timer tests and this protection test before uploading artifacts.

This remains a bootstrap mapping for QEMU `virt`, not process isolation. The next memory milestone is to split more RAM into fine-grained pages, add per-process address spaces and explicit user/kernel permissions, and integrate allocator ownership with page-table management. C code is compiled with general registers only until the kernel has a proper SIMD/FP context policy.
