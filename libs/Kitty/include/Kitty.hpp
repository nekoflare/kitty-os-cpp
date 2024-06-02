//
// Created by Piotr on 31.05.2024.
//

#ifndef KITTY_OS_CPP_KITTY_HPP
#define KITTY_OS_CPP_KITTY_HPP

inline void KernWrite(const char* buf) {
    asm volatile (
            "movq %0, %%rbx\n\t"   // Move buf to rbx (64-bit register)
            "movq $0x01, %%rax\n\t" // Move syscall number (0x01) to rax (64-bit register)
            "int $0x81\n\t"       // Trigger the interrupt
            :
            : "r" (buf)
            : "rax", "rbx"
            );
}


#endif //KITTY_OS_CPP_KITTY_HPP
