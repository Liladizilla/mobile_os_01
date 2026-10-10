# Dedus Mobile OS

[![ARM64 kernel CI](https://github.com/Liladizilla/mobile_os_01/actions/workflows/arm64-kernel.yml/badge.svg?branch=feat%2Faarch64-real-kernel)](https://github.com/Liladizilla/mobile_os_01/actions/workflows/arm64-kernel.yml)

Dedus is a low-level ARM64 operating-system research project written in freestanding C and AArch64 assembly. The repository also contains an older browser-based UI prototype. That prototype is for UX exploration only; it is not a native GUI running on the kernel.

## Current status

| Component | Status |
|---|---|
| ARM64 startup and EL2-to-EL1 transition | Implemented for QEMU `virt` |
| PL011 UART and interactive serial shell | Implemented |
| Flattened device-tree RAM discovery | Implemented for QEMU `virt` |
| Bitmap-backed 4 KiB physical page allocator | Implemented; `memtest` verifies allocation and release |
| EL1 exception vectors and panic diagnostics | Implemented and smoke-tested |
| Generic virtual timer + GICv2 IRQ path | Implemented and CI-tested; `ticks` confirms interrupts are delivered |
| EL1 MMU and cache enablement | Implemented and CI-tested on QEMU; 4 KiB kernel pages enforce RO executable text, RO+XN rodata and RW+XN data |
| Process address spaces and user/kernel isolation | Not implemented yet |
| Scheduler, user space, filesystem, native graphics/input | Not implemented yet |
| Retail-phone boot support | Not implemented or verified |

## Build and run

You need an AArch64 cross-compiler/binutils and QEMU system emulation. The CI environment uses `aarch64-linux-gnu-gcc` and `qemu-system-aarch64`.

```sh
make arm64
make run-arm64
```

Or build directly:

```sh
make -C kernel/arm64 CROSS_COMPILE=aarch64-linux-gnu- all
```

The kernel targets the QEMU `virt` board with a GICv2 interrupt controller and a PL011 serial console. Shell commands include `help`, `uname`, `features`, `meminfo`, `memtest`, `mmu`, `timer`, `ticks`, `vectors`, `about`, `fault`, and `protectfault`.

## CI/CD

GitHub Actions builds the freestanding kernel, boots it in QEMU, checks RAM discovery and allocator behavior, verifies timer interrupt delivery and exception diagnostics, validates the browser prototype's JavaScript and Dedus branding, and uploads the built kernel artifacts after the smoke test succeeds.

## UI/UX direction

Dedus follows **solid morphism**: solid tactile surfaces, crisp shadow outlines, restrained depth, and optional blur rather than glass effects everywhere. The mobile experience is designed around reachable controls, clear Back/Home/Recents behavior, accessible touch targets, scalable text, reduced motion, and graceful performance degradation. Read [the UX principles](docs/UX_PRINCIPLES.md).

The browser prototype must not present simulated hardware telemetry, permissions, encryption, or security features as implemented kernel capabilities.

## Roadmap

- [x] ARM64 entry, linker layout and serial shell
- [x] FDT-based RAM discovery and initial physical page allocator
- [x] EL1 exception vector table and panic diagnostics
- [x] Generic virtual timer IRQ delivery through GICv2 (QEMU `virt`)
- [ ] Process address spaces, allocator integration, and user/kernel isolation
- [ ] Scheduler, threads, context switching and system calls
- [ ] Virtual filesystem and block storage
- [ ] Native display/input stack and compositor
- [ ] Select and investigate a specific phone/SoC boot chain and drivers

## Why QEMU first?

ARM64 is an instruction-set family, not a promise that one image boots on every phone. A retail port needs a device-specific boot chain, SoC configuration, device tree, display/input/storage/network/power drivers, and a tested recovery path. No retail-phone image is provided. Do not flash an unverified kernel over an existing Android installation.

## Project and design documents

- [Real mobile OS plan](docs/REAL_MOBILE_OS_PLAN.md)
- [UI/UX principles](docs/UX_PRINCIPLES.md)
- [Draft implementation PR](https://github.com/Liladizilla/mobile_os_01/pull/1)
- [GitHub Actions](https://github.com/Liladizilla/mobile_os_01/actions)
- [Portfolio project registry](https://github.com/Liladizilla/portfolio/blob/main/data/project-links.json)
