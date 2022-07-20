intel8080
=========

An emulator for the Intel 8080 microprocessor written in C++.

I have run this on multiple test suites; it passes some but not all of them completely. I'm unsure why despite hours of debugging. Feel free to contribute!

The header file [intel8080.hpp](src/intel8080.hpp) is fairly well-documented. You should be able to figure out how to run the emulator from there (let me know if not). Of note:
- You can change and get the value of registers: `cpu.A()`, `cpu.B()`, `cpu.C()`, etc. as well as `cpu.PC`, `cpu.SP`.
- `intel8080::cpu::step()` executes the next instruction. If there is an interrupt waiting it runs that instead of the next instruction in memory.
- `intel8080::cpu::interrupt()` interrupts the CPU, but it does not actually run the interrupt vector; for that you must run `step()` afterwards.
- `intel8080::cpu::getHalted()` tells you whether the CPU is halted. This is important to check for; if you keep running `step()` while this returns true, it will simply do nothing.

It is your job to combine these and the other given functions in a way that will allow the CPU to run as you wish, either step-by-step or continuously (a loop is handy).

I may make improvements on this and provide code examples in the future. Again, feel free to contribute and thanks for reading.