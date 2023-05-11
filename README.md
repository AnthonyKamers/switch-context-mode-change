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

## Notes
The code is not working due to MMU mistakes. It gives illegal
instruction when try to set the SATP register in boot.s:86. As this
is not the goal of the current job (as it is to make the process
context switch), this has not been fixed, as it would take long time
to solve all the potential issues from the code used (from 2020.2).

Whereas this is not working, the code is still here to show the
approach that was taken to solve the process context switch. If you
take it off everything from supervisor mode and execute in machine mode,
obviously the MMU will not set (as it is not in supervisor mode), but
you can go through the code to know and be aware of how to correctly
switch the context between two processes. For this to be testes is
also necessary to switch from sifive_u to virt (you can do that by
uncommenting the respective lines in the Makefile), or just go to
the commit `6e7522ad` and execute it.

It has been presented in 24/04/2023 for Giovani and we explained the
whole situation and difficulty in setting the supervisor mode on and
the potential problems in the MMU (which is not part of the current
project!). We also pointed out at the presentation that it is not
necessary to make the switch context through timer interruption,
but we CHOSE to do that in THIS project only to exemplify where and
how we make the process context switch!


## Execute the project
To start it:
```bash
make clean && make && make start
```

or to debug it:
```bash
make clean && make && make debug
```