#include <dev/framebuffer.hpp>
#include <limine.h>
#include <stddef.h>

volatile struct limine_framebuffer_request framebuffer = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 1,
    .response = nullptr,
};

struct limine_framebuffer **framebuffers = nullptr;
size_t framebuffer_count = 0;

void initialize_framebuffer(void)
{
    if (framebuffer.response == nullptr)
    {
        // Halt the processor. There's no framebuffer.
        asm volatile("cli; hlt");
    }

    framebuffers = framebuffer.response->framebuffers;
    framebuffer_count = framebuffer.response->framebuffer_count;

    if (framebuffer_count <= 0)
    {
        // Halt the processor. There's no framebuffer.
        asm volatile("cli; hlt");
    }
}

limine_framebuffer *get_framebuffer(size_t index)
{
    if (index > framebuffer_count)
    {
        return nullptr;
    }

    return framebuffers[index];
}

size_t get_framebuffer_count(void)
{
    return framebuffer_count;
}