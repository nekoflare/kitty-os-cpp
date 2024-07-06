//
// Created by Piotr on 02.05.2024.
//

#ifndef FB_HPP
#define FB_HPP

#include <limine.h>
#include <arch/x64/control/control.hpp>
#include <stdint.h>
#include <sys/types.h>
#include <ccmath/basic.hpp>

namespace Framebuffer
{
    extern limine_framebuffer** _Framebuffers;
    extern limine_framebuffer* _MainFramebuffer;
    extern volatile limine_framebuffer_request _FramebuffersRequest;
    extern size_t _Fbcount;

    void Initialize();
    limine_framebuffer* GetFramebuffer(size_t _FbIdx);
    void DrawPixel(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b);
    uint32_t GetPixel(size_t _FbIdx, size_t xpos, size_t ypos);
    void DrawRectangle(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, size_t rectw, size_t recth);
    void DrawFilledRectangle(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, size_t rectw, size_t recth);
    void DrawCircle(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, size_t radius);
    void DrawFilledCircle(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, size_t radius);
    void DrawLine(size_t _FbIdx, size_t x0, size_t y0, size_t x1, size_t y1, uint8_t r, uint8_t g, uint8_t b);
    void DrawPixelAlpha(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
}

#endif //FB_HPP
