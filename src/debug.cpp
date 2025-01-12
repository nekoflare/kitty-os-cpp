#include <debug.hpp>
#include <dev/framebuffer.hpp>
#include <flanterm/backends/fb.h>
#include <flanterm/flanterm.h>
#include <hal/io.hpp>
#include <string.h>
#include <sys/nanoprintf.hpp>
#include <sys/symbols.hpp>

struct flanterm_context *ft_ctx = nullptr;

bool is_flanterm_initialized = false;

void initialize_flanterm()
{
    auto fb = get_framebuffer(0);

    ft_ctx = flanterm_fb_init(NULL, NULL, reinterpret_cast<uint32_t *>(fb->address), fb->width, fb->height, fb->pitch,
                              fb->red_mask_size, fb->red_mask_shift, fb->green_mask_size, fb->green_mask_shift,
                              fb->blue_mask_size, fb->blue_mask_shift, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              0, 0, 1, 0, 0, 0);

    is_flanterm_initialized = true;
}

void flanterm_putc(char c)
{
    if (is_flanterm_initialized)
        flanterm_write(ft_ctx, &c, 1);
}

void flanterm_puts(const char *s)
{
    if (is_flanterm_initialized)
        flanterm_write(ft_ctx, s, strlen(s));
}

void debug_putc(char c)
{
    outb(0xe9, c);
}

void debug_puts(const char *s)
{
    while (*s != '\0')
    {
        debug_putc(*s);
        s++;
    }
}

void debug_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    // Use nanoprintf to format the string
    char buffer[1024]; // Adjust buffer size as needed
    npf_vsnprintf(buffer, sizeof(buffer), fmt, args);

    // Print the formatted string using debug_puts
    debug_puts(buffer);
    flanterm_puts(buffer);

    va_end(args);
}

void debug_printf_va_list(const char *fmt, va_list args)
{
    char buffer[1024];
    npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
    debug_puts(buffer);
    flanterm_puts(buffer);
}

EXPORT_SYMBOL(debug_putc)
EXPORT_SYMBOL(debug_puts)
EXPORT_SYMBOL(debug_printf)
EXPORT_SYMBOL(debug_printf_va_list)