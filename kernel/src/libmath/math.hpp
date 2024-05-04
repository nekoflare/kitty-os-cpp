//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_MATH_HPP
#define KITTY_OS_CPP_MATH_HPP

#include <type_traits>

namespace Math
{
    template<typename T>
    typename std::enable_if<std::is_arithmetic<T>::value && std::is_signed<T>::value, T>::type
    abs(T v)
    {
        return (v < 0) ? -v : v;
    }

    template<typename T>
    const T& min(const T& a, const T& b) {
        return (b < a) ? b : a;
    }
}


#endif //KITTY_OS_CPP_MATH_HPP
