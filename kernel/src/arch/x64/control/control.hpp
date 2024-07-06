#ifndef CONTROL_H
#define CONTROL_H

extern "C" void unreachable();
inline void bochs_breakpoint()
{
    __asm__ volatile ("xchg %%bx, %%bx" ::: "bx");
}

#endif