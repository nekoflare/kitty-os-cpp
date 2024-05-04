//
// Created by Piotr on 03.05.2024.
//

#include "kstdio.hpp"
#include <kernel/fb/fb.hpp>
#include <kstd/kstring.hpp>
#include <cstdarg>

namespace kstd
{
    flanterm_context *ft_ctx = nullptr;

    void InitializeTerminal()
    {
        limine_framebuffer* main_framebuffer = Framebuffer::GetFramebuffer(0);
        if (main_framebuffer == nullptr) return;

        ft_ctx = flanterm_fb_init(
                NULL,
                NULL,
                (unsigned int*)main_framebuffer->address, main_framebuffer->width, main_framebuffer->height, main_framebuffer->pitch,
                main_framebuffer->red_mask_size, main_framebuffer->red_mask_shift,
                main_framebuffer->green_mask_size, main_framebuffer->green_mask_shift,
                main_framebuffer->blue_mask_size, main_framebuffer->blue_mask_shift,
                NULL,
                NULL, NULL,
                NULL, NULL,
                NULL, NULL,
                NULL, 0, 0, 1,
                0, 0,
                0
        );
    }

    void puts(const char* s)
    {
        flanterm_write(ft_ctx, s, kstd::strlen(s));
    }

    void putc(const char c)
    {
        flanterm_write(ft_ctx, (const char*)&c, 1);
    }

    void print_char(const char c)
    {
        putc(c);
    }

    void print_short(signed short s) {
        // Print the sign of the number
        if (s < 0) {
            putc('-');
            s = -s; // Make it positive for printing
        }

        // Convert each digit to character and print
        if (s == 0) {
            putc('0');
            return;
        }

        // Temporary buffer to hold digits
        char buffer[10];
        int i = 0;

        // Extract digits
        while (s != 0) {
            buffer[i++] = '0' + (s % 10);
            s /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--) {
            putc(buffer[j]);
        }
    }

    void print_hex_short(unsigned short s) {
        putc('0');
        putc('x');

        // Convert each nibble to hexadecimal digit and print
        for (int i = sizeof(s) * 2 - 1; i >= 0; i--) {
            int nibble = (s >> (i * 4)) & 0xF; // Extract each nibble
            char hex_digit = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10); // Convert to hexadecimal digit
            putc(hex_digit);
        }
    }

    void print_unsigned_short(unsigned short us) {
        // Convert each nibble to hexadecimal digit and print
        for (int i = sizeof(us) * 2 - 1; i >= 0; i--) {
            int nibble = (us >> (i * 4)) & 0xF; // Extract each nibble
            char hex_digit = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10); // Convert to hexadecimal digit
            putc(hex_digit);
        }
    }

    void print_long(signed long l) {
        // Print the sign of the number
        if (l < 0) {
            putc('-');
            l = -l; // Make it positive for printing
        }

        // Convert each digit to character and print
        if (l == 0) {
            putc('0');
            return;
        }

        // Temporary buffer to hold digits
        char buffer[20]; // Assuming maximum number of digits in a long
        int i = 0;

        // Extract digits
        while (l != 0) {
            buffer[i++] = '0' + (l % 10);
            l /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--) {
            putc(buffer[j]);
        }
    }

    void print_long_long(signed long long ll) {
        // Print the sign of the number
        if (ll < 0) {
            putc('-');
            ll = -ll; // Make it positive for printing
        }

        // Convert each digit to character and print
        if (ll == 0) {
            putc('0');
            return;
        }

        // Temporary buffer to hold digits
        char buffer[20]; // Assuming maximum number of digits in a long long
        int i = 0;

        // Extract digits
        while (ll != 0) {
            buffer[i++] = '0' + (ll % 10);
            ll /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--) {
            putc(buffer[j]);
        }
    }

    void print_unsigned_long(unsigned long ul) {
        // Convert each digit to character and print
        if (ul == 0) {
            putc('0');
            return;
        }

        // Temporary buffer to hold digits
        char buffer[20]; // Assuming maximum number of digits in an unsigned long
        int i = 0;

        // Extract digits
        while (ul != 0) {
            buffer[i++] = '0' + (ul % 10);
            ul /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--) {
            putc(buffer[j]);
        }
    }

    void print_unsigned_long_long(unsigned long long ull) {
        // Convert each digit to character and print
        if (ull == 0) {
            putc('0');
            return;
        }

        // Temporary buffer to hold digits
        char buffer[20]; // Assuming maximum number of digits in an unsigned long long
        int i = 0;

        // Extract digits
        while (ull != 0) {
            buffer[i++] = '0' + (ull % 10);
            ull /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--) {
            putc(buffer[j]);
        }
    }

    void print_hex_char(unsigned char c) {
        // Convert each nibble to hexadecimal digit and print
        for (int i = sizeof(c) * 2 - 1; i >= 0; i--) {
            int nibble = (c >> (i * 4)) & 0xF; // Extract each nibble
            char hex_digit = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10); // Convert to hexadecimal digit
            putc(hex_digit);
        }
    }

    void print_hex_long(unsigned long l) {
        // Convert each nibble to hexadecimal digit and print
        for (int i = sizeof(l) * 2 - 1; i >= 0; i--) {
            int nibble = (l >> (i * 4)) & 0xF; // Extract each nibble
            char hex_digit = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10); // Convert to hexadecimal digit
            putc(hex_digit);
        }
    }

    void print_hex_long_long(unsigned long long ll) {
        // Convert each nibble to hexadecimal digit and print
        for (int i = sizeof(ll) * 2 - 1; i >= 0; i--) {
            int nibble = (ll >> (i * 4)) & 0xF; // Extract each nibble
            char hex_digit = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10); // Convert to hexadecimal digit
            putc(hex_digit);
        }
    }

    void print_double(double d) {
        // Handle negative numbers
        if (d < 0) {
            putc('-');
            d = -d;
        }

        // Print integer part
        unsigned long long int_part = (unsigned long long)d;
        print_unsigned_long_long(int_part);
        putc('.');

        // Print decimal part
        double decimal_part = d - int_part;
        for (int i = 0; i < 6; ++i) { // Print up to 6 decimal places
            decimal_part *= 10;
            int digit = (int)decimal_part;
            putc('0' + digit);
            decimal_part -= digit;
        }
    }

    void printf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

        char c;
        unsigned char uc;
        unsigned short us;
        signed short ss;
        unsigned long ul;
        signed long sl;
        unsigned long long ull;
        signed long long sll;
        char* strptr;
        double d;

        while (*fmt)
        {
            switch (*fmt)
            {
                case '%':
                    fmt++;
                    switch (*fmt) // %..
                    {
                        case 'h':
                            fmt++;
                            switch(*fmt)
                            {
                                case 'h':
                                    fmt++;
                                    switch (*fmt)
                                    {
                                        case 'x': //hhx
                                            uc = static_cast<unsigned char>(va_arg(args, int)); // promote char -> int
                                            print_hex_char(uc);
                                            break;
                                        case 'd': // hhd
                                            sl = va_arg(args, int); // char -> int
                                            print_long(sl);
                                            break;
                                        case 'u': // hhu
                                            ul = va_arg(args, unsigned int); // char -> uint
                                            print_unsigned_long(ul);
                                            break;
                                        default:
                                            putc(*fmt);
                                            break;
                                    }

                                    break;
                                case 'x':
                                    us = static_cast<unsigned short>(va_arg(args, int)); // promote short -> int
                                    print_hex_short(us);
                                    break;
                                case 'd':
                                    ss = static_cast<signed short>(va_arg(args, int)); // promote short -> int
                                    print_short(ss);
                                    break;
                                case 'u':
                                    us = static_cast<unsigned short>(va_arg(args, int)); // promote short -> int
                                    print_unsigned_short(us);
                                    break;
                                default:
                                    putc(*fmt);
                                    break;
                            }
                            break;
                        case 'l':
                            fmt++;
                            switch (*fmt)
                            {
                                case 'l':
                                    fmt++;
                                    switch(*fmt)
                                    {
                                        case 'x':
                                            ull = va_arg(args, unsigned long long);
                                            print_hex_long_long(ull);
                                            break;
                                        case 'd':
                                            sll = va_arg(args, long long);
                                            print_long_long(sll);
                                            break;
                                        case 'u':
                                            ull = va_arg(args, unsigned long long);
                                            print_unsigned_long_long(ull);
                                            break;
                                        default:
                                            putc(*fmt);
                                            break;
                                    }
                                    break;
                                case 'x':
                                    ul = va_arg(args, unsigned long);
                                    print_hex_long(ul);
                                    break;
                                case 'd':
                                    sl = va_arg(args, long);
                                    print_long(sl);
                                    break;
                                case 'u':
                                    ul = va_arg(args, unsigned long);
                                    print_unsigned_long(ul);
                                    break;
                                default:
                                    putc(*fmt);
                                    break;
                            }
                            break;
                        case 's':
                            strptr = va_arg(args, char *);
                            puts(strptr);
                            break;
                        case 'c':
                            c = static_cast<char>(va_arg(args, int));
                            putc(c);
                            break;
                        case 'f':
                            d = va_arg(args, double); // promotion float -> double. if double, no promotion.
                            print_double(d);
                            break;
                        default:
                            putc(*fmt);
                            break;
                    }
                    break;
                default:
                    break;
            }
            putc(*fmt);
            fmt++;
        }

        va_end(args);
    }

}