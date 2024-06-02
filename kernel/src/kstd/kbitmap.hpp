//
// Created by Piotr on 06.05.2024.
//

#ifndef KITTY_OS_CPP_KBITMAP_HPP
#define KITTY_OS_CPP_KBITMAP_HPP

#include <stdint.h>
#include <stddef.h>
#include <kstd/kstdio.hpp>

template <typename T>
class Bitmap
{
protected:
    T* gBitmap;
    size_t gSize;
    size_t last_free_block = 0;
public:
    // Set the bit in the bitmap
    void Set(size_t idx)
    {
        if ((idx / sizeof(T)) > this->gSize) return;

        this->gBitmap[idx / sizeof(T)] |= (1 << (idx % sizeof(T)));
    }

    // Toggle the bit in the bitmap (If bit == 0, then bit = 1 and vice versa)
    void Toggle(size_t idx)
    {
        if ((idx / sizeof(T)) > this->gSize) return;

        this->gBitmap[idx / sizeof(T)] ^= (1 << (idx % sizeof(T)));
    }

    // Clears the bit
    void Clear(size_t idx)
    {
        if ((idx / sizeof(T)) > this->gSize) return;

        this->gBitmap[idx / sizeof(T)] &= ~(1 << (idx % sizeof(T)));
    }

    // Checks the bit (returns true if set, false if not set).
    bool Check(size_t idx)
    {
        if ((idx / sizeof(T)) > this->gSize) return false;

        return (this->gBitmap[idx / sizeof(T)] & (1 << (idx % sizeof(T)))) != 0;
    }

    // Returns number of set bits.
    size_t CountSet()
    {
        size_t count = 0;
        for (size_t i = 0; i < this->gSize * sizeof(T); ++i)
        {
            if (this->Check(i))
            {
                ++count;
            }
        }
        return count;
    }
    // Returns number of cleared bits.
    size_t CountUnset()
    {
        return (this->gSize * sizeof(T)) - this->CountSet();
    }

    // Returns index of first set bit.
    size_t FindFirstSet()
    {
        for (size_t i = 0; i < this->gSize * sizeof(T); ++i)
        {
            if (this->Check(i))
            {
                return i;
            }
        }
        return SIZE_MAX; // Indicate no set bit found
    }

    // Returns index of first unset bit.
    size_t FindFirstCleared()
    {
        if (last_free_block != 0)
        {
            for (size_t i = last_free_block; i < this->gSize * sizeof(T); ++i)
            {
                if (!this->Check(i))
                {
                    last_free_block = i;
                    return i;
                }
            }
        }
        else
        {
            for (size_t i = 0; i < this->gSize * sizeof(T); ++i)
            {
                if (!this->Check(i))
                {
                    last_free_block = i;
                    return i;
                }
            }
        }

        kstd::printf("NO BLOCK FOUND!!");
        unreachable();
        return 0; // Indicate no unset bit found
    }

    // Returns the index of the continuous block of set bits.
    size_t FindContinuousBlockOfSet(size_t length)
    {
        size_t consecutive = 0;
        for (size_t i = 0; i < this->gSize * sizeof(T); ++i)
        {
            if (this->Check(i))
            {
                ++consecutive;
                if (consecutive >= length)
                {
                    return i - consecutive + 1;
                }
            }
            else
            {
                consecutive = 0;
            }
        }
        return SIZE_MAX; // Indicate no continuous block of set bits found
    }

    // Returns the index of continuous block unset bits.
    size_t FindContinuousBlockOfUnset(size_t length)
    {
        size_t consecutive = 0;
        for (size_t i = 0; i < this->gSize * sizeof(T); ++i)
        {
            if (!this->Check(i))
            {
                ++consecutive;
                if (consecutive >= length)
                {
                    return i - consecutive + 1;
                }
            }
            else
            {
                consecutive = 0;
            }
        }
        return SIZE_MAX; // Indicate no continuous block of unset bits found
    }

    void ClearToOnes()
    {
        for (size_t i = 0; (this->gSize * sizeof(T)) > i; i++)
        {
            this->Set(i);
        }
    }

    void ClearToZeroes()
    {
        for (size_t i = 0; (this->gSize * sizeof(T)) > i; i++)
        {
            this->Clear(i);
        }
    }

    void Initialize(T* bitmap_pointer = nullptr, size_t bitmap_size = 0)
    {
        this->gBitmap = bitmap_pointer;
        this->gSize = bitmap_size;
    }
};

#endif //KITTY_OS_CPP_KBITMAP_HPP
