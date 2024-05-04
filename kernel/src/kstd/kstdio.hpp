//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_KSTDIO_HPP
#define KITTY_OS_CPP_KSTDIO_HPP

#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>

namespace kstd
{
    extern flanterm_context* ft_ctx;

    void InitializeTerminal();
    void puts(const char* s);
    void putc(const char c);

    void print_char(const char c);
    void print_short(signed short s);
    void print_long(signed long l);
    void print_long_long(signed long long ll);
    void print_unsigned_short(unsigned short us);
    void print_unsigned_long(unsigned long ul);
    void print_unsigned_long_long(unsigned long long ull);
    void print_hex_char(unsigned char c);
    void print_hex_short(unsigned short s);
    void print_hex_long(unsigned long l);
    void print_hex_long_long(unsigned long long ll);
    void print_double(double d);

    void printf(const char* fmt, ...) ;
}

#endif //KITTY_OS_CPP_KSTDIO_HPP
