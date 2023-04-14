FILE ?= test

RISCV64 := /usr/local/rv64/bin/riscv64-unknown-linux-gnu-
TERMINAL := xterm -e

SOURCES_ASM=$(wildcard asm/*.s)
SOURCES_C=$(wildcard c/*.c)
SOURCES_H=$(wildcard c/*.h)
SOURCES_MMU=$(wildcard mmu/*.c)
SOURCES_MMU_H=$(wildcard mmu/*.h)

LINKER=virt.ld

QEMU=qemu-system-riscv64
MACH=sifive_u

LIB= -lgcc


default:
	$(RISCV64)g++ -g -march=rv64g -mabi=lp64d -static -mcmodel=medany \
	-fvisibility=hidden -nostdlib -nostartfiles -T $(LINKER) -o $(FILE).bin \
	$(SOURCES_ASM) $(SOURCES_H) $(SOURCES_C) $(SOURCES_MMU_H) $(SOURCES_MMU) \
	$(LIB) -w

start:
	@qemu-system-riscv64 -nographic -machine virt -bios none -kernel $(FILE).bin

start_asm:
	@qemu-system-riscv64 -nographic -machine virt -bios none -kernel $(FILE).bin -d in_asm

clean:
	@rm -f *.bin

debug:
	@qemu-system-riscv64 -nographic -machine virt -bios none -kernel $(FILE).bin \
	-gdb tcp::1234 -S & $(TERMINAL) $(RISCV64)gdb -ex "target remote:1234" \
	-ex "set confirm off" -ex "add-symbol-file ./$(FILE).bin 0x80000000"

	lsof -t -i :1234 | xargs kill -9

boot-only:
	$(RISCV64)gcc -g -march=rv64g -mabi=lp64d -static -mcmodel=medany \
	-fvisibility=hidden -nostdlib -nostartfiles -Tvirt.ld -o boot.bin boot.s trap.s print.s
