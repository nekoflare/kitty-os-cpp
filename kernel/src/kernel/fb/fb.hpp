//
// Created by Piotr on 02.05.2024.
//

#ifndef FB_HPP
#define FB_HPP

#include <limine.h>
#include <control/control.hpp>
#include <kstd/kstdint.hpp>

namespace Framebuffer
{
    extern limine_framebuffer** _Framebuffers;
    extern limine_framebuffer* _MainFramebuffer;
    extern volatile limine_framebuffer_request _FramebuffersRequest;
    extern kstd::size_t _Fbcount;

    void Initialize();
    limine_framebuffer* GetFramebuffer(kstd::size_t _FbIdx);
    void DrawPixel(kstd::size_t _FbIdx, kstd::size_t xpos, kstd::size_t ypos, kstd::uint8_t r, kstd::uint8_t g, kstd::uint8_t b);
    kstd::uint32_t GetPixel(kstd::size_t _FbIdx, kstd::size_t xpos, kstd::size_t ypos);
    void DrawRectangle(kstd::size_t _FbIdx, kstd::size_t xpos, kstd::size_t ypos, kstd::uint8_t r, kstd::uint8_t g, kstd::uint8_t b, kstd::size_t rectw, kstd::size_t recth);
    void DrawFilledRectangle(kstd::size_t _FbIdx, kstd::size_t xpos, kstd::size_t ypos, kstd::uint8_t r, kstd::uint8_t g, kstd::uint8_t b, kstd::size_t rectw, kstd::size_t recth);
    void DrawCircle(kstd::size_t _FbIdx, kstd::size_t xpos, kstd::size_t ypos, kstd::uint8_t r, kstd::uint8_t g, kstd::uint8_t b, kstd::size_t radius);
    void DrawFilledCircle(kstd::size_t _FbIdx, kstd::size_t xpos, kstd::size_t ypos, kstd::uint8_t r, kstd::uint8_t g, kstd::uint8_t b, kstd::size_t radius);
    void DrawLine(kstd::size_t _FbIdx, kstd::size_t x0, kstd::size_t y0, kstd::size_t x1, kstd::size_t y1, kstd::uint8_t r, kstd::uint8_t g, kstd::uint8_t b);
};

#endif //FB_HPP
