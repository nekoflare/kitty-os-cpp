//
// Created by Piotr on 12.05.2024.
//

#ifndef KITTY_OS_CPP_SERIAL_HPP
#define KITTY_OS_CPP_SERIAL_HPP

#include <kdu/driver_entry.hpp>

constexpr uint64_t UART_CREATE_HANDLE = 0x0;
constexpr uint64_t UART_DESTROY_HANDLE = 0x1;

struct UartCreateHandle
{
    size_t com_port;
};

struct UartDestroyHandle
{
    size_t com_port;
};

#endif //KITTY_OS_CPP_SERIAL_HPP
