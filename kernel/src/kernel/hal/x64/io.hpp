//
// Created by Piotr on 05.05.2024.
//

#ifndef KITTY_OS_CPP_IO_HPP
#define KITTY_OS_CPP_IO_HPP

#include <kstd/kstdint.hpp>

// Input/Output operations for single-byte data
inline kstd::uint8_t inb(kstd::uint16_t port) {
    kstd::uint8_t data;
    asm volatile("inb %w1, %b0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outb(kstd::uint16_t port, kstd::uint8_t data) {
    asm volatile("outb %b0, %w1" : : "a"(data), "Nd"(port));
}

// Input/Output operations for double-byte data
inline kstd::uint16_t inw(kstd::uint16_t port) {
    kstd::uint16_t data;
    asm volatile("inw %w1, %w0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outw(kstd::uint16_t port, kstd::uint16_t data) {
    asm volatile("outw %w0, %w1" : : "a"(data), "Nd"(port));
}

// Input/Output operations for quad-byte data
inline kstd::uint32_t inl(kstd::uint16_t port) {
    kstd::uint32_t data;
    asm volatile("inl %w1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outl(kstd::uint16_t port, kstd::uint32_t data) {
    asm volatile("outl %0, %w1" : : "a"(data), "Nd"(port));
}

// Input/Output operations for 64-bit data
inline kstd::uint64_t inq(kstd::uint16_t port) {
    kstd::uint64_t data;
    asm volatile("inq %w1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

inline void outq(kstd::uint16_t port, kstd::uint64_t data) {
    asm volatile("outq %0, %w1" : : "a"(data), "Nd"(port));
}

inline void iowait(kstd::size_t t)
{
    for (kstd::size_t n = 0; t > n; n++)
        outb(0x80, 0x00);
}

#endif //KITTY_OS_CPP_IO_HPP
