//
// Created by Piotr on 27.05.2024.
//

#ifndef KITTY_OS_CPP_BV_HPP
#define KITTY_OS_CPP_BV_HPP

#include <kdu/driver_entry.hpp>
#include <kstd/kstdio.hpp>
#include <kdu/apis/serial.hpp>
#include <kernel/fb/fb.hpp>
#include <kdu/apis/graphics.hpp>
#include <kernel/hal/x64/io.hpp>

constexpr uint16_t VBE_DISPI_IOPORT_INDEX = 0x01CE;
constexpr uint16_t VBE_DISPI_IOPORT_DATA = 0x01CF;

constexpr size_t VBE_DISPI_INDEX_ID = 0;
constexpr size_t VBE_DISPI_INDEX_XRES = 1;
constexpr size_t VBE_DISPI_INDEX_YRES = 2;
constexpr size_t VBE_DISPI_INDEX_BPP = 3;
constexpr size_t VBE_DISPI_INDEX_ENABLE = 4;
constexpr size_t VBE_DISPI_INDEX_BANK = 5;
constexpr size_t VBE_DISPI_INDEX_VIRT_WIDTH = 6;
constexpr size_t VBE_DISPI_INDEX_VIRT_HEIGHT = 7;
constexpr size_t VBE_DISPI_INDEX_X_OFFSET = 8;
constexpr size_t VBE_DISPI_INDEX_Y_OFFSET = 9;



#endif //KITTY_OS_CPP_BV_HPP
