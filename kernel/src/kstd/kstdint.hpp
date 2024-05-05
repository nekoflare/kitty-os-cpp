//
// Created by Piotr on 02.05.2024.
//

#ifndef KSTDINT_HPP
#define KSTDINT_HPP

#define KSTD_SIZE_MAX 18446744073709551615

namespace kstd
{
    typedef unsigned char uchar_t;
    typedef signed char char_t;
    typedef unsigned char uint8_t;
    typedef signed char int8_t;

    typedef unsigned short ushort_t;
    typedef signed short short_t;
    typedef unsigned short uint16_t;
    typedef signed short int16_t;

    typedef unsigned long ulong_t;
    typedef signed long long_t;
    typedef unsigned long uint32_t;
    typedef signed long int32_t;

    typedef unsigned long long ulonglong_t;
    typedef signed long long longlong_t;
    typedef unsigned long long uint64_t;
    typedef signed long long int64_t;

    typedef unsigned long long size_t;
    typedef signed long long ssize_t;
}

#endif //KSTDINT_HPP
