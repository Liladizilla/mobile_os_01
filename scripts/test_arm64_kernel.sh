#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
QEMU="${QEMU:-qemu-system-aarch64}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
BUILD_DIR="$ROOT/kernel/arm64/build"
IMAGE="$BUILD_DIR/adizilla.elf"
LOG="$(mktemp)"
trap 'rm -f "$LOG"' EXIT

make -C "$ROOT/kernel/arm64" CROSS_COMPILE="$CROSS_COMPILE" "$IMAGE" >/dev/null
command -v "$QEMU" >/dev/null || { echo "Missing QEMU executable: $QEMU" >&2; exit 1; }

# QEMU is expected to keep running in the interactive shell, so timeout is normal.
set +e
printf 'help\nuname\nfeatures\nabout\n' | timeout 8s "$QEMU" \
  -machine virt -cpu cortex-a53 -m 256M -nographic -monitor none -kernel "$IMAGE" >"$LOG" 2>&1
status=$?
set -e
if [ "$status" -ne 124 ] && [ "$status" -ne 0 ]; then
  cat "$LOG"
  echo "QEMU exited unexpectedly with status $status" >&2
  exit "$status"
fi

grep -q 'ADIZILLA MOBILE OS' "$LOG"
grep -q 'Adizilla Mobile OS kernel 0.2.0' "$LOG"
grep -q 'PL011 UART input/output' "$LOG"
grep -q 'not yet installable on a retail phone' "$LOG"
echo "ARM64 kernel smoke test passed: boot banner, command loop and honesty checks verified."
