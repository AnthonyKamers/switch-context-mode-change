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

########################### sifive_u ###########################
#FLAGS_GCC:=-march=rv64gc -mabi=lp64d -Wl, -mno-relax -mcmodel=medany
#MACHINE=sifive_u
#CPUS=2
#LIB=
#FLAGS_QEMU=-ex "target extended-remote:1234" -ex "add-inferior" -ex "inferior 2" -ex "attach 2"

########################### virt ###########################
FLAGS_GCC:=-march=rv64g -mabi=lp64d -static -mcmodel=medany
MACHINE=virt
CPUS=4
LIB= -lgcc
FLAGS_QEMU=-ex "target remote:1234"


default:
	$(RISCV64)g++ -g $(FLAGS_GCC) \
	-fvisibility=hidden -nostdlib -nostartfiles -T $(LINKER) -o $(FILE).bin \
	$(SOURCES_ASM) $(SOURCES_H) $(SOURCES_C) $(SOURCES_MMU_H) $(SOURCES_MMU) \
	$(LIB) -w

start:
	$(QEMU) -nographic -machine $(MACHINE) -smp $(CPUS) -bios none -kernel $(FILE).bin

clean:
	@rm -f *.bin

debug:
	$(QEMU) -nographic -machine $(MACHINE) -smp $(CPUS) -bios none -kernel $(FILE).bin -D log.log -d mmu,int \
	-gdb tcp::1234 -S & $(TERMINAL) $(RISCV64)gdb $(FLAGS_QEMU) \
	-ex "set confirm off" \
	-ex "add-symbol-file ./$(FILE).bin 0x80000000"

	lsof -t -i :1234 | xargs kill -9