//
// Created by Neko on 25.01.2025.
//

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <cstddef>
#include <cstdint>
#include <kernel.h>

void initialize_framebuffer();
bool framebuffer_exists();
handle get_framebuffer(size_t framebuffer_index);

void draw_pixel(handle framebuffer_handle, uint64_t x, uint64_t y, uint64_t r, uint64_t g, uint64_t b);
void draw_rectangle(handle framebuffer_handle, uint64_t x, uint64_t y, uint64_t width, uint64_t height, uint64_t r,
                    uint64_t g, uint64_t b);
void draw_bitmap(const handle framebuffer_handle, const uint8_t *buffer, int width, int height, int channels);

#endif // FRAMEBUFFER_H
