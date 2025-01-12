//
// Created by Piotr on 02.01.2025.
//

#include <cstdint>

extern "C" {
    unsigned int __popcountdi2(uint64_t x) {
        unsigned int count = 0;
        while (x) {
            count += x & 1;
            x >>= 1;
        }
        return count;
    }
}
