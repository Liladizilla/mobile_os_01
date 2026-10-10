# Adizilla Mobile OS

Adizilla is being rebuilt as a **real low-level ARM64 operating system project**. The repository also contains an older browser-based UI prototype. The prototype is useful for exploring the user experience, but it is not itself a phone OS and its simulated hardware/security features must not be mistaken for kernel functionality.

## Current reality

| Component | Status |
|---|---|
| Freestanding ARM64 kernel | Initial implementation in `kernel/arm64/` |
| AArch64 assembly entry point and linker layout | Implemented for the QEMU `virt` board |
| PL011 UART console | Implemented |
| Interactive serial command loop | Implemented |
| QEMU boot / physical phone boot | QEMU target is the first test target; retail phone boot is **not implemented or verified** |
| MMU, real scheduler, userspace, storage, touchscreen, GPU, Wi-Fi, modem, camera, audio, power management | Roadmap |
| Browser UI and built-in app mockups | Legacy prototype, not kernel services |

## Build the real ARM64 kernel

Install an AArch64 bare-metal GCC/binutils toolchain and QEMU system emulator. On Fedora, package names can vary by release; verify availability before installing. Then run:

```sh
make arm64
make run-arm64
```

Or:

```sh
make -C kernel/arm64
make -C kernel/arm64 run
```

QEMU opens a serial console. Available commands: `help`, `uname`, `features`, `clear`, and `about`.

The first kernel is freestanding C plus AArch64 assembly. It talks directly to the QEMU PL011 UART registers; it does not use Node.js, a browser, or hosted C libraries in the kernel. This is a small, testable foundation, not a finished mobile OS.

## Why QEMU first?

ARM64 is an instruction-set family, not a guarantee that one image boots on every phone. A retail phone needs a device-specific boot chain, SoC configuration, device tree, display/input/storage/network/power drivers, and a recovery plan. The intended physical target must be selected and investigated before any image is flashed. No existing Android partition should be overwritten with an unverified build.

## Roadmap

### Kernel and hardware foundations
- [x] AArch64 startup assembly and linker script
- [x] PL011 serial input/output and basic command shell on QEMU `virt`
- [ ] Exception vectors, timer interrupts and interrupt controller
- [ ] RAM map discovery, page allocator, MMU and process isolation
- [ ] Preemptive scheduler, processes/threads and system calls
- [ ] Virtual filesystem, block storage and persistent settings
- [ ] Device-tree parsing and a real device-specific boot target

### Mobile platform
- [ ] Display/GPU composition, touchscreen and gestures
- [ ] Battery, charging, thermal limits, suspend/resume and power management
- [ ] Wi-Fi, cellular modem, Bluetooth, audio, camera, GPS and sensors
- [ ] Secure boot/update integration and rollback/recovery
- [ ] User-space service manager, permission model and application runtime

### User experience and prototype feature carry-over
These are product goals to implement on top of working kernel and user-space services, not claims that they already work in the kernel:
- Launcher/home screen, task switcher, notifications and lock screen
- File manager, settings, terminal, browser, text editor, calculator
- Themes, accessibility, responsive motion, blur/shadows where hardware supports them
- User accounts, permissions, audit logs, sandboxing and secure app installation
- CPU/memory/storage/network/battery monitoring backed by actual device telemetry
- Notes, tasks, calendar, timers and alarms
- Weather, news, crypto prices, dictionary, currency conversion and NASA content as user-space network apps
- AI assistant with explicit network/credential permissions
- GitHub developer tools and safe terminal workflows
- Optional Swahili/localization and region-specific service integrations where supported

Security items such as encryption, firewalling, intrusion detection, 2FA, vulnerability scanning and malware scanning must use real, reviewed implementations and be independently tested before being advertised as available.

## Repository layout

- `kernel/arm64/`: new freestanding ARM64 kernel target
- `kernel/` and `asm/`: older experimental code; not the new ARM64 kernel
- `frontend/` and `backend/`: browser prototype and its supporting server
- `scripts/`, `iso/`, and the checked-in legacy ISO: older experimental build path. Do not treat it as a verified phone image.
- `docs/REAL_MOBILE_OS_PLAN.md`: architecture notes, honest status and milestone plan

## Contribution rule

Keep kernel code freestanding and low-level. UI and API-backed features belong in user space after the kernel provides the required services. Mark features as implemented only after they are exercised in emulation or on supported hardware.
