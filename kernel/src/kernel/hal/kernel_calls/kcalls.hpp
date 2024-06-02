//
// Created by Piotr on 31.05.2024.
//

#ifndef KITTY_OS_CPP_KCALLS_HPP
#define KITTY_OS_CPP_KCALLS_HPP

#include "../x64/idt/idt.hpp"

void kcall_handler(Registers_x86_64* regs);

#endif //KITTY_OS_CPP_KCALLS_HPP
