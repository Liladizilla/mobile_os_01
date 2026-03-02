# Adizilla OS - Assembly Makefile

NASM = nasm
ASFLAGS = -f elf32

SOURCES = boot.asm syscall.asm interrupt.asm
OBJECTS = $(SOURCES:.asm=.o)

TARGET = boot_sector.bin

.PHONY: all clean

all: $(TARGET)

boot_sector.bin: boot.o syscall.o interrupt.o
	@echo "[LD] Creating boot sector..."
	@ld -m elf_i386 -o $@ $^

boot.o: boot.asm
	@echo "[AS] $@"
	$(NASM) $(ASFLAGS) $< -o $@

syscall.o: syscall.asm
	@echo "[AS] $@"
	$(NASM) $(ASFLAGS) $< -o $@

interrupt.o: interrupt.asm
	@echo "[AS] $@"
	$(NASM) $(ASFLAGS) $< -o $@

clean:
	@echo "[CLEAN] Removing build files..."
	rm -f $(OBJECTS) $(TARGET)
