//
// Created by Piotr on 31.05.2024.
//

#include "kcstdlib.hpp"



void SysExit(int exitCode) {
    asm volatile (
            "movl %0, %%ebx\n\t"  // Move exitCode to ebx (32-bit register)
            "movq $0x00, %%rax\n\t" // Move syscall number (0x00) to rax (64-bit register)
            "int $0x81\n\t"       // Trigger the interrupt
            :
            : "r" (exitCode)
            : "rax", "rbx"
            );
}
