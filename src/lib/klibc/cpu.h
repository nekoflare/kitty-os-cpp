//
// Created by Neko on 19.02.2025.
//

#ifndef CPU_H
#define CPU_H
#include <cstdint>

extern "C"
{
    int __popcountdi2(uint64_t x);
}

#endif // CPU_H
