//
// Created by Piotr on 05.05.2024.
//

#ifndef KITTY_OS_CPP_CPUID_HPP
#define KITTY_OS_CPP_CPUID_HPP

#include <stdint.h>

inline void cpuid(uint32_t function, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx)
{
    asm volatile ("cpuid"
            : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
            : "a" (function));
}

#endif //KITTY_OS_CPP_CPUID_HPP
