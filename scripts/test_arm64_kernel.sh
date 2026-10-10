#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
QEMU="${QEMU:-qemu-system-aarch64}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
IMAGE="$ROOT/kernel/arm64/build/dedus.bin"
LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT
make -C "$ROOT/kernel/arm64" CROSS_COMPILE="$CROSS_COMPILE" all >/dev/null
command -v "$QEMU" >/dev/null || { echo "Missing QEMU executable: $QEMU" >&2; exit 1; }
set +e
(sleep 1; printf 'help\nuname\nfeatures\nmeminfo\nmemtest\nmmu\ntimer\nticks\nabout\nvectors\nfault\n') | timeout 12s "$QEMU" -machine virt,gic-version=2 -cpu cortex-a53 -m 256M -nographic -monitor none -kernel "$IMAGE" >"$LOG" 2>&1
status=$?
set -e
if [ "$status" -ne 124 ] && [ "$status" -ne 0 ]; then cat "$LOG"; echo "QEMU exited unexpectedly with status $status" >&2; exit "$status"; fi
for expected in 'DEDUS MOBILE OS' 'Dedus Mobile OS kernel 0.4.0' 'EL1 exception vectors installed' 'EL1 MMU and caches enabled with identity mappings.' 'MMU status: enabled' 'FDT RAM discovery' 'Dedus physical memory report' 'Physical page allocator self-test: PASS' 'Generic virtual timer IRQ and GICv2 initialized' 'Timer IRQ count:' 'not yet installable on a retail phone' 'Vector base check: PASS' 'DEDUS KERNEL EXCEPTION' 'ESR_EL1:'; do
  if ! grep -q "$expected" "$LOG"; then cat "$LOG"; echo "Smoke test failed: expected output not found: $expected" >&2; exit 1; fi
done
if ! grep -Eq 'Timer IRQ count: 0x000000000000000[1-9a-f]' "$LOG"; then
  cat "$LOG"
  echo "Smoke test failed: timer interrupt count did not advance" >&2
  exit 1
fi
echo "Dedus ARM64 smoke test passed: boot, FDT memory, page allocation, GICv2 timer IRQs, shell and exception diagnostics."
