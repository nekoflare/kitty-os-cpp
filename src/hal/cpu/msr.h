//
// Created by Neko on 21.01.2025.
//

#ifndef MSR_H
#define MSR_H
#include <cstdint>

static inline void wrmsr(uint32_t msr, uint64_t value)
{
    asm volatile("wrmsr" : : "c"(msr), "a"(static_cast<uint32_t>(value)), "d"(static_cast<uint32_t>(value >> 32)));
}

static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return (static_cast<uint64_t>(high) << 32) | low;
}

#endif // MSR_H
