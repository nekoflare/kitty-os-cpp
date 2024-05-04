//
// Created by Piotr on 03.05.2024.
//

#include "kstring.hpp"

namespace kstd
{
    kstd::size_t strlen(const char* s)
    {
        kstd::size_t len = 0;
        while (*s++ != 0)
        {
            len++;
        }
        return len;
    }

    void* memset(void* ptr, int v, kstd::size_t num)
    {
        char* _Ptr = static_cast<char*>(ptr);
        char _Value = static_cast<char>(v); // Cast v to char

        while (num-- > 0)
        {
            *_Ptr++ = _Value; // Increment _Ptr to move to the next memory location
        }

        return _Ptr; // Return the end pointer
    }

    void* memcpy(void* dest, const void* src, kstd::size_t num)
    {
        char* _Dest = static_cast<char*>(dest);
        const char* _Src = static_cast<const char*>(src);

        // Copy byte by byte
        for (kstd::size_t i = 0; i < num; ++i) {
            _Dest[i] = _Src[i];
        }

        return dest;
    }

    void* memmove(void* dest, const void* src, kstd::size_t num)
    {
        char* _Dest = static_cast<char*>(dest);
        const char* _Src = static_cast<const char*>(src);

        // If source and destination addresses overlap
        if (_Dest > _Src && _Dest < _Src + num) {
            // Copy from end to start to avoid overwriting
            for (kstd::size_t i = num; i != 0; --i)
            {
                _Dest[i - 1] = _Src[i - 1];
            }
        } else {
            // Copy from start to end
            for (kstd::size_t i = 0; i < num; ++i)
            {
                _Dest[i] = _Src[i];
            }
        }

        return dest;
    }


    int memcmp(const void* ptr1, const void* ptr2, kstd::size_t num)
    {
        const unsigned char* p1 = static_cast<const unsigned char*>(ptr1);
        const unsigned char* p2 = static_cast<const unsigned char*>(ptr2);

        for (kstd::size_t i = 0; i < num; ++i)
        {
            if (p1[i] != p2[i])
            {
                return (p1[i] < p2[i]) ? -1 : 1;
            }
        }
        return 0;
    }

    void* memchr(const void* ptr, int value, kstd::size_t num)
    {
        const unsigned char* p = static_cast<const unsigned char*>(ptr);
        for (kstd::size_t i = 0; i < num; ++i)
        {
            if (p[i] == value)
            {
                return const_cast<void*>(static_cast<const void*>(p + i));
            }
        }
        return nullptr;
    }

    void* memrchr(const void* ptr, int value, kstd::size_t num)
    {
        const unsigned char* p = static_cast<const unsigned char*>(ptr);
        for (kstd::size_t i = num; i > 0; --i)
        {
            if (p[i - 1] == value)
            {
                return const_cast<void*>(static_cast<const void*>(p + i - 1));
            }
        }
        return nullptr;
    }

    void* memccpy(void* dest, const void* src, int c, kstd::size_t num)
    {
        unsigned char* d = static_cast<unsigned char*>(dest);
        const unsigned char* s = static_cast<const unsigned char*>(src);

        for (kstd::size_t i = 0; i < num; ++i)
        {
            d[i] = s[i];
            if (s[i] == static_cast<unsigned char>(c))
            {
                return d + i + 1;
            }
        }
        return nullptr;
    }

    void memswap(void* ptr1, void* ptr2, kstd::size_t num)
    {
        unsigned char* p1 = static_cast<unsigned char*>(ptr1);
        unsigned char* p2 = static_cast<unsigned char*>(ptr2);

        for (kstd::size_t i = 0; i < num; ++i)
        {
            unsigned char temp = p1[i];
            p1[i] = p2[i];
            p2[i] = temp;
        }
    }
}
