//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_KSTRING_HPP
#define KITTY_OS_CPP_KSTRING_HPP

#include <kstd/kstdint.hpp>

namespace kstd
{
    kstd::size_t strlen(const char* s);
    void* memset(void* ptr, int v, kstd::size_t num);
    void* memcpy(void* dest, const void* src, kstd::size_t num);
    void* memmove(void* dest, const void* src, kstd::size_t num);
    int memcmp(const void* ptr1, const void* ptr2, kstd::size_t num);
    void* memchr(const void* ptr, int value, kstd::size_t num);
    void* memrchr(const void* ptr, int value, kstd::size_t num);
    void* memccpy(void* dest, const void* src, int c, kstd::size_t num);
    void memswap(void* ptr1, void* ptr2, kstd::size_t num);
}

#endif //KITTY_OS_CPP_KSTRING_HPP