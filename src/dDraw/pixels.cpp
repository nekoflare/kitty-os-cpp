#include <dDraw/pixels.hpp>

static inline uint8_t *get_pixel_address(limine_framebuffer *framebuffer, size_t x, size_t y)
{
    return static_cast<uint8_t *>(framebuffer->address) + y * framebuffer->pitch + x * (framebuffer->bpp / 8);
}

void draw_pixel(limine_framebuffer *framebuffer, size_t x, size_t y, unsigned int r, unsigned int g, unsigned int b)
{
    if (x >= framebuffer->width || y >= framebuffer->height)
    {
        return;
    }

    uint8_t *pixel_address = get_pixel_address(framebuffer, x, y);

    uint64_t pixel_value = 0;
    pixel_value |= ((r & ((1 << framebuffer->red_mask_size) - 1)) << framebuffer->red_mask_shift);
    pixel_value |= ((g & ((1 << framebuffer->green_mask_size) - 1)) << framebuffer->green_mask_shift);
    pixel_value |= ((b & ((1 << framebuffer->blue_mask_size) - 1)) << framebuffer->blue_mask_shift);

    size_t bytes_per_pixel = framebuffer->bpp / 8;
    for (size_t i = 0; i < bytes_per_pixel; ++i)
    {
        pixel_address[i] = (pixel_value >> (i * 8)) & 0xFF;
    }
}

void draw_rectangle(limine_framebuffer *framebuffer, size_t x, size_t y, size_t w, size_t h, unsigned int r,
                    unsigned int g, unsigned int b)
{
    if (x >= framebuffer->width || y >= framebuffer->height)
    {
        return;
    }
    size_t max_width = (x + w > framebuffer->width) ? framebuffer->width - x : w;
    size_t max_height = (y + h > framebuffer->height) ? framebuffer->height - y : h;

    uint64_t pixel_value = 0;
    pixel_value |= ((r & ((1 << framebuffer->red_mask_size) - 1)) << framebuffer->red_mask_shift);
    pixel_value |= ((g & ((1 << framebuffer->green_mask_size) - 1)) << framebuffer->green_mask_shift);
    pixel_value |= ((b & ((1 << framebuffer->blue_mask_size) - 1)) << framebuffer->blue_mask_shift);

    size_t bytes_per_pixel = framebuffer->bpp / 8;

    for (size_t row = 0; row < max_height; ++row)
    {
        uint8_t *row_address = get_pixel_address(framebuffer, x, y + row);
        for (size_t col = 0; col < max_width; ++col)
        {
            uint8_t *pixel_address = row_address + col * bytes_per_pixel;
            for (size_t i = 0; i < bytes_per_pixel; ++i)
            {
                pixel_address[i] = (pixel_value >> (i * 8)) & 0xFF;
            }
        }
    }
}