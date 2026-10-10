CROSS_COMPILE ?= aarch64-none-elf-
CC := $(CROSS_COMPILE)gcc
QEMU ?= qemu-system-aarch64
BUILD := build
TARGET := $(BUILD)/dedus.elf
CFLAGS := -I../include -mcpu=cortex-a53 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fno-pie -nostdlib -nostartfiles -Wall -Wextra -O2
LDFLAGS := -T linker.ld -nostdlib -Wl,--build-id=none -Wl,-Map,$(BUILD)/dedus.map
.PHONY: all run clean
all: $(TARGET)
$(BUILD):
	mkdir -p $(BUILD)
$(BUILD)/boot.o: boot.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/vectors.o: vectors.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/kernel.o: kernel.c ../include/memory.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/memory.o: memory.c ../include/memory.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@
$(TARGET): $(BUILD)/boot.o $(BUILD)/vectors.o $(BUILD)/kernel.o $(BUILD)/memory.o linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUILD)/boot.o $(BUILD)/vectors.o $(BUILD)/kernel.o $(BUILD)/memory.o -o $@
	@echo "Built Dedus freestanding ARM64 kernel: $(TARGET)"
run: $(TARGET)
	$(QEMU) -machine virt -cpu cortex-a53 -m 256M -nographic -monitor none -kernel $(TARGET)
clean:
	rm -rf $(BUILD)
