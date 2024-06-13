//
// Created by Piotr on 05.06.2024.
//

#ifndef KITTY_OS_CPP_PIT_HPP
#define KITTY_OS_CPP_PIT_HPP

#include <kernel/hal/x64/io.hpp>

constexpr uint16_t PIT_CHANNEL_0 = 0x40;
constexpr uint16_t PIT_COMMAND = 0x43;
constexpr size_t PIT_FREQUENCY = 1193182;

inline void pit_init(const size_t desired_frequency)
{
    const uint16_t divisor = PIT_FREQUENCY / desired_frequency;

    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL_0, divisor & 0xff);
    outb(PIT_CHANNEL_0, (divisor >> 8) & 0xff);
}

#endif //KITTY_OS_CPP_PIT_HPP
