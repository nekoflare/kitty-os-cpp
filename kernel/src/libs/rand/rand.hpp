//
// Created by Piotr on 15.06.2024.
//

#ifndef KITTY_OS_CPP_RAND_HPP
#define KITTY_OS_CPP_RAND_HPP

#include <type_traits>
#include <cstdint>

// Linear Congruential Generator (LCG) parameters
constexpr uint64_t a = 1664525;
constexpr uint64_t c = 1013904223;
constexpr uint64_t m = 4294967296; // 2^32

unsigned long lcg();

// Templated random function with enable_if for integral and floating-point types
template <typename T, typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value, int>::type = 0>
T rand(T x, T y) {
    unsigned long rand_max = m - 1;
    unsigned long randomValue = lcg();

    if constexpr (std::is_floating_point<T>::value) {
        double scaledValue = static_cast<double>(randomValue) / rand_max;
        return static_cast<T>(x + scaledValue * (y - x));
    } else {
        return x + (randomValue % (y - x + 1));
    }
}

#endif //KITTY_OS_CPP_RAND_HPP
