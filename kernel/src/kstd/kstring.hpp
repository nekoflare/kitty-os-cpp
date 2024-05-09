//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_KSTRING_HPP
#define KITTY_OS_CPP_KSTRING_HPP

#include <stdint.h>
#include <sys/types.h>

namespace kstd
{
    size_t strlen(const char* s);
    void* memset(void* ptr, int v, size_t num);
    void* memcpy(void* dest, const void* src, size_t num);
    void* memmove(void* dest, const void* src, size_t num);
    int memcmp(const void* ptr1, const void* ptr2, size_t num);
    void* memchr(const void* ptr, int value, size_t num);
    void* memrchr(const void* ptr, int value, size_t num);
    void* memccpy(void* dest, const void* src, int c, size_t num);
    void memswap(void* ptr1, void* ptr2, size_t num);
}

#endif //KITTY_OS_CPP_KSTRING_HPP