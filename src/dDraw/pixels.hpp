#ifndef _PIXELS_HPP
#define _PIXELS_HPP

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

void draw_pixel(limine_framebuffer *framebuffer, size_t x, size_t y, unsigned int r, unsigned int g, unsigned int b);
void draw_rectangle(limine_framebuffer *framebuffer, size_t x, size_t y, size_t w, size_t h, unsigned int r,
                    unsigned int g, unsigned int b);

#endif