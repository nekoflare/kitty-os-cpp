//
// Created by Piotr on 19.05.2024.
//

#ifndef EMULATOR_UTILS_HPP
#define EMULATOR_UTILS_HPP

template <typename T>
constexpr T KB(T v)
{
    return v * 1024;
}

template <typename T>
constexpr T MB(T v)
{
    return v * 1024 * 1024;
}

template <typename T>
constexpr T GB(T v)
{
    return v * 1024 * 1024 * 1024;
}

template <typename T>
constexpr T TB(T v)
{
    return v * 1024 * 1024 * 1024 * 1024;
}

#endif //EMULATOR_UTILS_HPP
