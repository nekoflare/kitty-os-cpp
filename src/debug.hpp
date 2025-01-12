#pragma once

#include <stdarg.h>

/*
 * Those functions report to port 0xE9 data.
 */
void debug_putc(char c);
void debug_puts(const char *s);
void debug_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void debug_printf_va_list(const char *fmt, va_list args);
void initialize_flanterm();