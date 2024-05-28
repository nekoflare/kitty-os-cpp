//
// Created by Piotr on 26.05.2024.
//

#ifndef KITTY_OS_CPP_PC_UART_HPP
#define KITTY_OS_CPP_PC_UART_HPP

#include <kdu/driver_entry.hpp>
#include <kstd/kstdio.hpp>
#include <kdu/apis/serial.hpp>

struct Handles
{
    driver_handle_t driverHandle;
    size_t com_port;
};

#endif //KITTY_OS_CPP_PC_UART_HPP
