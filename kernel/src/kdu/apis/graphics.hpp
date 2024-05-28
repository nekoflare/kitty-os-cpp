//
// Created by Piotr on 13.05.2024.
//

#ifndef KITTY_OS_CPP_GRAPHICS_HPP
#define KITTY_OS_CPP_GRAPHICS_HPP

constexpr uint64_t GPU_SET_RESOLUTION = 0x0;

struct GpuResolution
{
    size_t width;
    size_t height;
    size_t bpp;
};

#endif //KITTY_OS_CPP_GRAPHICS_HPP
