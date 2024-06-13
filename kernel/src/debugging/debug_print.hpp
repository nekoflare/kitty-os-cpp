//
// Created by Piotr on 06.06.2024.
//

#ifndef KITTY_OS_CPP_DEBUG_PRINT_HPP
#define KITTY_OS_CPP_DEBUG_PRINT_HPP

#include "uart.hpp"

void dbg_init();
void dbg_write_str(const char* msg);
void dbg_write_chr(const char msg);

#endif //KITTY_OS_CPP_DEBUG_PRINT_HPP
