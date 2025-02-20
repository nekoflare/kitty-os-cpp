//
// Created by Neko on 20.01.2025.
//

#include "log.h"
#include <cstdarg>
#include <lib/npf/nanoprintf.h>

#include <hal/cpu/io.h>

void debug_puts(const char *msg)
{
    for (; *msg != '\0'; msg++)
    {
        outb(0xE9, *msg);
    }
}

void debug_print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buffer[1024];
    npf_vsnprintf(buffer, sizeof(buffer), fmt, args);

    debug_puts(buffer);

    va_end(args);
}