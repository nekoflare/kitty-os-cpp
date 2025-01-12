#pragma once

extern "C" void debug_putc(char c);
extern "C" void debug_puts(const char *s);
extern "C" void debug_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));