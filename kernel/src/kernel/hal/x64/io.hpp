//
// Created by Piotr on 05.05.2024.
//

#ifndef KITTY_OS_CPP_IO_HPP
#define KITTY_OS_CPP_IO_HPP

#include <stdint.h>
#include <sys/types.h>

// Input/Output operations for single-byte data
inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %b0, %w1" : : "a"(data), "Nd"(port));
}

// Input/Output operations for double-byte data
inline uint16_t inw(uint16_t port) {
    uint16_t data;
    asm volatile("inw %w1, %w0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outw(uint16_t port, uint16_t data) {
    asm volatile("outw %w0, %w1" : : "a"(data), "Nd"(port));
}

// Input/Output operations for quad-byte data
inline uint32_t inl(uint16_t port) {
    uint32_t data;
    asm volatile("inl %w1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outl(uint16_t port, uint32_t data) {
    asm volatile("outl %0, %w1" : : "a"(data), "Nd"(port));
}

inline void iowait(size_t t)
{
    for (size_t n = 0; t > n; n++)
        outb(0x80, 0x00);
}

#endif //KITTY_OS_CPP_IO_HPP
