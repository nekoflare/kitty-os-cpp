//
// Created by Piotr on 05.05.2024.
//

#ifndef KITTY_OS_CPP_CPUID_HPP
#define KITTY_OS_CPP_CPUID_HPP

#include <stdint.h>

void cpuid(int function, int& eax, int& ebx, int& ecx, int& edx)
{
    asm volatile ("cpuid"
            : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
            : "a" (function));
}

#endif //KITTY_OS_CPP_CPUID_HPP
