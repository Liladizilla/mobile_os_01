# Adizilla OS - Kernel Makefile

CC = gcc
CFLAGS = -m32 -Wall -nostdinc -fno-pie -no-pie
ASFLAGS = --32
LDFLAGS = -m32 -nostdlib

SOURCES = adz_core.c adz_mem.c adz_proc.c adz_net.c
OBJECTS = $(SOURCES:.c=.o)

TARGET = adz_kernel.bin

.PHONY: all clean

all: $(TARGET)

adz_kernel.bin: $(OBJECTS)
	@echo "[LD] Linking kernel..."
	@ld $(LDFLAGS) -o $@ $^

adz_core.o: adz_core.c include/adz.h
	@echo "[CC] $@"
	$(CC) $(CFLAGS) -c $< -o $@

adz_mem.o: adz_mem.c include/adz.h
	@echo "[CC] $@"
	$(CC) $(CFLAGS) -c $< -o $@

adz_proc.o: adz_proc.c include/adz.h
	@echo "[CC] $@"
	$(CC) $(CFLAGS) -c $< -o $@

adz_net.o: adz_net.c include/adz.h
	@echo "[CC] $@"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "[CLEAN] Removing build files..."
	rm -f $(OBJECTS) $(TARGET)
