//
// Created by Piotr on 13.05.2024.
//

#ifndef KITTY_OS_CPP_GRAPHICS_HPP
#define KITTY_OS_CPP_GRAPHICS_HPP

enum GraphicsAPIIoctls
{
    GPU_SET_RESOLUTION,
    GPU_CREATE_RENDER_OBJECT,
    GPU_DESTROY_RENDER_OBJECT,
    GPU_UPDATE_RENDER_OBJECT,
    GPU_WRITE_TO_RENDER_OBJECT
};

struct GfxCreateRenderObjectReq
{
    size_t width, height, bpp;
};


struct GpuResolution
{
    size_t width;
    size_t height;
    size_t bpp;
};

#endif //KITTY_OS_CPP_GRAPHICS_HPP
