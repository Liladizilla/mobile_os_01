CROSS_COMPILE ?= aarch64-none-elf-
CC := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
QEMU ?= qemu-system-aarch64
BUILD := build
TARGET := $(BUILD)/adizilla.elf

CFLAGS := -mcpu=cortex-a53 -ffreestanding -fno-builtin -fno-stack-protector \
          -fno-pic -fno-pie -nostdlib -nostartfiles -Wall -Wextra -O2
LDFLAGS := -T linker.ld -nostdlib -Wl,--build-id=none -Wl,-Map,$(BUILD)/adizilla.map

.PHONY: all run clean check-tools

all: $(TARGET)

check-tools:
	@command -v $(CC) >/dev/null || { echo "Missing $(CC). Install an AArch64 bare-metal GCC toolchain."; exit 1; }
	@command -v $(QEMU) >/dev/null || { echo "Missing $(QEMU). Install QEMU system emulator."; exit 1; }

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.o: kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(BUILD)/boot.o $(BUILD)/kernel.o linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUILD)/boot.o $(BUILD)/kernel.o -o $@
	@echo "Built freestanding ARM64 kernel: $(TARGET)"

run: $(TARGET)
	$(QEMU) -machine virt -cpu cortex-a53 -m 256M -nographic -monitor none -kernel $(TARGET)

clean:
	rm -rf $(BUILD)
