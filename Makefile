.PHONY: all kernel asm clean install start

all: install kernel asm
	@echo "✅ Adizilla MOB OS built successfully"

install:
	npm install

kernel:
	$(MAKE) -C kernel/

asm:
	$(MAKE) -C asm/

start:
	npm start

dev:
	npm run dev

clean:
	$(MAKE) -C kernel/ clean
	$(MAKE) -C asm/ clean
	rm -f logs/*.log

logs:
	tail -f logs/system.log