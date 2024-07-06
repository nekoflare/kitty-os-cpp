//
// Created by Piotr on 13.05.2024.
//

#ifndef KITTY_OS_CPP_VMWARE_SVGAII_HPP
#define KITTY_OS_CPP_VMWARE_SVGAII_HPP

#include <public/kdu/driver_entry.hpp>
#include <kstd/kstdio.hpp>
#include <public/kdu/apis/serial.hpp>
#include <drivers/video/fb/fb.hpp>
#include <public/kdu/apis/graphics.hpp>
#include <hal/x64/io.hpp>
#include <hal/bus/pci.hpp>
#include <mm/vmm.hpp>
#include <arch/x64/control/control.hpp>

constexpr uint32_t SVGA_INDEX = 0;
constexpr uint32_t SVGA_VALUE = 1;
constexpr uint32_t SVGA_BIOS = 2;
constexpr uint32_t SVGA_IRQSTATUS = 8;

constexpr uint32_t SVGA_REG_ID = 0;
constexpr uint32_t SVGA_REG_ENABLE = 1;
constexpr uint32_t SVGA_REG_WIDTH = 2;
constexpr uint32_t SVGA_REG_HEIGHT = 3;
constexpr uint32_t SVGA_REG_MAX_WIDTH = 4;
constexpr uint32_t SVGA_REG_MAX_HEIGHT = 5;
constexpr uint32_t SVGA_REG_BPP = 7;
constexpr uint32_t SVGA_REG_FB_START = 13;
constexpr uint32_t SVGA_REG_FB_OFFSET = 14;
constexpr uint32_t SVGA_REG_VRAM_SIZE = 15;
constexpr uint32_t SVGA_REG_FB_SIZE = 16;
constexpr uint32_t SVGA_REG_CAPABILITIES = 17;
constexpr uint32_t SVGA_REG_FIFO_START = 18;
constexpr uint32_t SVGA_REG_FIFO_SIZE = 19;
constexpr uint32_t SVGA_REG_CONFIG_DONE = 20;
constexpr uint32_t SVGA_REG_SYNC = 21;
constexpr uint32_t SVGA_REG_BUSY = 22;

#endif //KITTY_OS_CPP_VMWARE_SVGAII_HPP
