.PHONY: all arm64 run-arm64 legacy-kernel asm clean install start dev logs

# The default build is now the freestanding ARM64 kernel, not the browser prototype.
all: arm64

arm64:
	$(MAKE) -C kernel/arm64

run-arm64: arm64
	$(MAKE) -C kernel/arm64 run

# The browser UI remains available for product/UX work only.
install:
	npm install

start:
	npm start

dev:
	npm run dev

# Older experimental x86 code is preserved but is not the ARM64 kernel.
legacy-kernel:
	$(MAKE) -C kernel/

asm:
	$(MAKE) -C asm/

clean:
	$(MAKE) -C kernel/arm64 clean
	$(MAKE) -C kernel/ clean
	$(MAKE) -C asm/ clean
	rm -f logs/*.log

logs:
	tail -f logs/system.log
