//
// Created by Neko on 19.02.2025.
//

#include "cpu.h"

#include <cstdint>

extern "C"
{
    int __popcountdi2(uint64_t x)
    {
        int count = 0;
        while (x)
        {
            count += x & 1;
            x >>= 1;
        }
        return count;
    }
}
