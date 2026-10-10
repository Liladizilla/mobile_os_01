#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
QEMU="${QEMU:-qemu-system-aarch64}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
IMAGE="$ROOT/kernel/arm64/build/adizilla.elf"
LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT

make -C "$ROOT/kernel/arm64" CROSS_COMPILE="$CROSS_COMPILE" all >/dev/null
command -v "$QEMU" >/dev/null || { echo "Missing QEMU executable: $QEMU" >&2; exit 1; }

# QEMU should remain in the interactive shell; timeout is therefore normal.
# The deliberate fault is last because the exception handler halts the kernel.
set +e
printf 'help\nuname\nfeatures\nabout\nfault\n' | timeout 8s "$QEMU" \
  -machine virt -cpu cortex-a53 -m 256M -nographic -monitor none -kernel "$IMAGE" >"$LOG" 2>&1
status=$?
set -e
if [ "$status" -ne 124 ] && [ "$status" -ne 0 ]; then
  cat "$LOG"
  echo "QEMU exited unexpectedly with status $status" >&2
  exit "$status"
fi

grep -q 'ADIZILLA MOBILE OS' "$LOG"
grep -q 'Adizilla Mobile OS kernel 0.3.0' "$LOG"
grep -q 'EL1 exception vectors installed' "$LOG"
grep -q 'PL011 UART input/output' "$LOG"
grep -q 'not yet installable on a retail phone' "$LOG"
grep -q 'ADIZILLA KERNEL EXCEPTION' "$LOG"
grep -q 'ESR_EL1:' "$LOG"
echo "ARM64 kernel smoke test passed: boot, serial commands and deliberate exception diagnostics verified."
