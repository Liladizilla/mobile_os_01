#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
QEMU="${QEMU:-qemu-system-aarch64}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
IMAGE="$ROOT/kernel/arm64/build/dedus.bin"
LOG="$(mktemp)"
PROTECT_LOG="$(mktemp)"
trap 'rm -f "$LOG" "$PROTECT_LOG"' EXIT
make -C "$ROOT/kernel/arm64" CROSS_COMPILE="$CROSS_COMPILE" all >/dev/null
command -v "$QEMU" >/dev/null || { echo "Missing QEMU executable: $QEMU" >&2; exit 1; }
set +e
(sleep 1; printf 'help\nuname\nfeatures\nmeminfo\nmemtest\nvmtest\nmmu\ntimer\nticks\nabout\nvectors\nfault\n') | timeout 12s "$QEMU" -machine virt,gic-version=2 -cpu cortex-a53 -m 256M -nographic -monitor none -kernel "$IMAGE" >"$LOG" 2>&1
status=$?
set -e
if [ "$status" -ne 124 ] && [ "$status" -ne 0 ]; then cat "$LOG"; echo "QEMU exited unexpectedly with status $status" >&2; exit "$status"; fi
for expected in 'DEDUS MOBILE OS' 'Dedus Mobile OS kernel 0.4.0' 'EL1 exception vectors installed' 'EL1 MMU and caches enabled with identity mappings.' 'MMU status: enabled' 'FDT RAM discovery' 'Dedus physical memory report' 'Physical page allocator self-test: PASS' 'VM address-space self-test: PASS' 'Generic virtual timer IRQ and GICv2 initialized' 'Timer IRQ count:' 'not yet installable on a retail phone' 'Vector base check: PASS' 'DEDUS KERNEL EXCEPTION' 'ESR_EL1:'; do
  if ! grep -q "$expected" "$LOG"; then cat "$LOG"; echo "Smoke test failed: expected output not found: $expected" >&2; exit 1; fi
done
if ! grep -Eq 'Timer IRQ count: 0x000000000000000[1-9a-f]' "$LOG"; then
  cat "$LOG"
  echo "Smoke test failed: timer interrupt count did not advance" >&2
  exit 1
fi
# Reboot separately and verify a write to the read-only text mapping raises a data abort.
set +e
(sleep 1; printf 'protectfault\n') | timeout 5s "$QEMU" -machine virt,gic-version=2 -cpu cortex-a53 -m 256M -nographic -monitor none -kernel "$IMAGE" >"$PROTECT_LOG" 2>&1
protect_status=$?
set -e
if [ "$protect_status" -ne 124 ] && [ "$protect_status" -ne 0 ]; then cat "$PROTECT_LOG"; echo "QEMU protection test exited unexpectedly with status $protect_status" >&2; exit "$protect_status"; fi
if ! grep -q 'DEDUS KERNEL EXCEPTION' "$PROTECT_LOG" || ! grep -q 'FAR_EL1: 0x000000004008' "$PROTECT_LOG"; then
  cat "$PROTECT_LOG"
  echo "MMU protection test failed: writing kernel text did not fault at the expected address" >&2
  exit 1
fi
echo "Dedus ARM64 smoke tests passed: boot, FDT memory, page allocation, MMU activation, read-only text protection, GICv2 timer IRQs, shell and exception diagnostics."
