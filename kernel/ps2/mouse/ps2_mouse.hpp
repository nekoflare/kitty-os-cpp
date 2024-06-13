//
// Created by Piotr on 03.06.2024.
//

#ifndef KITTY_OS_CPP_PS2_MOUSE_HPP
#define KITTY_OS_CPP_PS2_MOUSE_HPP

#include <kstd/kstdio.hpp>

#define PS2_MOUSE_DATA_PORT 0x60
#define PS2_MOUSE_COMMAND_PORT 0x64

#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08

void ps2_mouse_init();

#endif //KITTY_OS_CPP_PS2_MOUSE_HPP
