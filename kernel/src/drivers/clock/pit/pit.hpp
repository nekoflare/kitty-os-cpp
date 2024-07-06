//
// Created by Piotr on 05.06.2024.
//

#ifndef KITTY_OS_CPP_PIT_HPP
#define KITTY_OS_CPP_PIT_HPP

#include <hal/x64/io.hpp>

constexpr uint16_t PIT_CHANNEL_0 = 0x40;
constexpr uint16_t PIT_COMMAND = 0x43;
constexpr size_t PIT_FREQUENCY = 1193182;

void pit_init(const size_t desired_frequency);

#endif //KITTY_OS_CPP_PIT_HPP
