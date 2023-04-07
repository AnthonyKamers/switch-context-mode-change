# Switch Context - Mode Change (RiscV 64-bits)

In order to test the execution of this project,
it is necessary to have installed:
* qemu-system-riscv64
* toolchain riscv64
  * It is also necessary to change the actual location of
  your toolchain in Makefile::3
* xterm*
  * Only if you want to debug it
  * You can change it to the terminal of your choice in
  Makefile::4



To start it:
```bash
make clean && make && make start
```

or to debug it:
```bash
make clean && make && make debug
```