//
// Created by Neko on 25.01.2025.
//

#include "framebuffer.h"

#include <dbg/log.h>
#include <limine.h>
#include <vector>

limine_framebuffer_request fb_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 1, .response = nullptr};

std::vector<limine_framebuffer *> native_framebuffer;

bool framebuffer_exists()
{
    if (fb_request.response == nullptr)
        return false;

    if (fb_request.response->framebuffer_count == 0)
        return false;

    return true;
}

void initialize_framebuffer()
{
    if (fb_request.response == nullptr)
    {
        debug_print("No framebuffer available.\n");
    }

    const auto fb_count = fb_request.response->framebuffer_count;
    const auto fb_entries = fb_request.response->framebuffers;
    for (uint64_t i = 0; fb_count > i; i++)
    {
        const auto entry = fb_entries[i];
        native_framebuffer.push_back(entry);
    }
}

handle get_framebuffer(const size_t framebuffer_index)
{
    if (framebuffer_index >= native_framebuffer.size())
    {
        return nullptr;
    }

    return native_framebuffer[framebuffer_index];
}

static uint8_t *get_pixel_address(const limine_framebuffer *framebuffer, const size_t x, const size_t y)
{
    return static_cast<uint8_t *>(framebuffer->address) + y * framebuffer->pitch + x * (framebuffer->bpp / 8);
}

void draw_pixel(const handle framebuffer_handle, const uint64_t x, const uint64_t y, const uint64_t r, const uint64_t g,
                const uint64_t b)
{
    const auto framebuffer = static_cast<limine_framebuffer *>(framebuffer_handle);

    if (x >= framebuffer->width || y >= framebuffer->height)
    {
        return;
    }

    uint8_t *pixel_address = get_pixel_address(framebuffer, x, y);

    uint64_t pixel_value = 0;
    pixel_value |= (r & (1 << framebuffer->red_mask_size) - 1) << framebuffer->red_mask_shift;
    pixel_value |= (g & (1 << framebuffer->green_mask_size) - 1) << framebuffer->green_mask_shift;
    pixel_value |= (b & (1 << framebuffer->blue_mask_size) - 1) << framebuffer->blue_mask_shift;

    const size_t bytes_per_pixel = framebuffer->bpp / 8;
    for (size_t i = 0; i < bytes_per_pixel; ++i)
    {
        pixel_address[i] = pixel_value >> i * 8 & 0xFF;
    }
}

void draw_rectangle(const handle framebuffer_handle, const uint64_t x, const uint64_t y, uint64_t width,
                    uint64_t height, const uint64_t r, const uint64_t g, const uint64_t b)
{
    const auto framebuffer = static_cast<limine_framebuffer *>(framebuffer_handle);

    if (x + width > framebuffer->width)
    {
        width = framebuffer->width - x;
    }
    if (y + height > framebuffer->height)
    {
        height = framebuffer->height - y;
    }

    for (uint64_t i = 0; i < height; ++i)
    {
        for (uint64_t j = 0; j < width; ++j)
        {
            draw_pixel(framebuffer_handle, x + j, y + i, r, g, b);
        }
    }
}

void draw_bitmap(const handle framebuffer_handle, const uint8_t *buffer, int width, int height, int channels)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int index = (y * width + x) * channels;
            uint64_t r = buffer[index];
            uint64_t g = buffer[index + 1];
            uint64_t b = buffer[index + 2];
            draw_pixel(framebuffer_handle, x, y, r, g, b);
        }
    }
}
