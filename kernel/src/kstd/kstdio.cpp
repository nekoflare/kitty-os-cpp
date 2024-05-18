//
// Created by Piotr on 03.05.2024.
//

#include "kstdio.hpp"
#include <kernel/fb/fb.hpp>
#include <kstd/kstring.hpp>
#include <cstdarg>
#include <kernel/hal/x64/io.hpp>

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

    void print_signed_integer(signed int si)
    {
        // Handle negative numbers
        if (si < 0)
        {
            putc('-');
            si = -si;
        }

        // Extract digits and print them
        unsigned int place = 1;
        while (si / place >= 10)
            place *= 10;

        while (place > 0)
        {
            putc('0' + si / place);
            si %= place;
            place /= 10;
        }
        putc('\n');
    }

    void print_unsigned_integer_octal(unsigned int si)
    {
        // Handle the base case of 0
        if (si == 0) {
            putc('0');
            return;
        }

        // Array to store octal digits
        char octal_digits[32];
        int i = 0;

        // Extract octal digits
        while (si != 0)
        {
            octal_digits[i++] = '0' + (si % 8);
            si /= 8;
        }

        // Print octal digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(octal_digits[j]);
        }
    }


    void print_unsigned_integer_hexadecimal(unsigned int si)
    {
        // Handle the base case of 0
        if (si == 0)
        {
            putc('0');
            return;
        }

        // Array to store hexadecimal digits
        char hex_digits[32];
        int i = 0;

        // Extract hexadecimal digits
        while (si != 0)
        {
            int remainder = si % 16;
            if (remainder < 10)
                hex_digits[i++] = '0' + remainder; // Convert to ASCII
            else
                hex_digits[i++] = 'A' + (remainder - 10); // Convert to ASCII
            si /= 16;
        }

        // Print hexadecimal digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(hex_digits[j]);
        }
    }

    void print_unsigned_integer(unsigned int ui)
    {
        // Handle the base case of 0
        if (ui == 0)
        {
            putc('0');
            return;
        }

        // Array to store digits
        char digits[32];
        int i = 0;

        // Extract digits
        while (ui != 0)
        {
            digits[i++] = '0' + (ui % 10); // Convert to ASCII
            ui /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(digits[j]);
        }
    }

    void print_double(double d)
    {
        // Handle negative numbers
        if (d < 0) {
            putc('-');
            d = -d;
        }

        // Print integer part
        unsigned int integerPart = (unsigned int)d;
        print_unsigned_integer(integerPart);

        // Print decimal point
        putc('.');

        // Print fractional part
        const int maxPrecision = 15; // Maximum precision supported by double
        int precision = 0;
        while (precision < maxPrecision)
        {
            d -= integerPart; // Subtract integer part
            d *= 10; // Move next decimal place
            integerPart = (unsigned int)d; // Get next integer part
            putc('0' + integerPart); // Print next digit
            precision++;
            // Check if fractional part is zero
            if (ccm::fabs(d - integerPart) < 1e-10) // Tolerance for floating point comparison
                break;
        }
    }

    void print_pointer(void* ptr)
    {
        unsigned long long int address = (unsigned long long int)ptr; // Convert pointer to unsigned integer

        putc('0');
        putc('x');

        // Print pointer address in hexadecimal format
        // Print higher 32 bits
        unsigned int higher_bits = (unsigned int)(address >> 32);
        for (int i = 28; i >= 0; i -= 4)
        {
            unsigned int hex_digit = (higher_bits >> i) & 0xF;
            if (hex_digit < 10)
                putc('0' + hex_digit);
            else
                putc('A' + (hex_digit - 10));
        }

        // Print lower 32 bits
        unsigned int lower_bits = (unsigned int)(address & 0xFFFFFFFF);
        for (int i = 28; i >= 0; i -= 4)
        {
            unsigned int hex_digit = (lower_bits >> i) & 0xF;
            if (hex_digit < 10)
                putc('0' + hex_digit);
            else
                putc('A' + (hex_digit - 10));
        }
    }

    void print_long(long l)
    {
        // Handle negative numbers
        if (l < 0)
        {
            putc('-');
            l = -l;
        }

        // Handle the base case of 0
        if (l == 0)
        {
            putc('0');
            return;
        }

        // Array to store digits
        char digits[32];
        int i = 0;

        // Extract digits
        while (l != 0)
        {
            digits[i++] = '0' + (l % 10); // Convert to ASCII
            l /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(digits[j]);
        }
    }

    void print_unsigned_long_octal(unsigned long ul)
    {
        // Handle the base case of 0
        if (ul == 0)
        {
            putc('0');
            return;
        }

        // Array to store octal digits
        char octal_digits[32];
        int i = 0;

        // Extract octal digits
        while (ul != 0)
        {
            octal_digits[i++] = '0' + (ul % 8); // Convert to ASCII
            ul /= 8;
        }

        // Print octal digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(octal_digits[j]);
        }
    }

    void print_unsigned_long_hexadecimal(unsigned long ul)
    {
        // Handle the base case of 0
        if (ul == 0)
        {
            putc('0');
            return;
        }

        // Array to store hexadecimal digits
        char hex_digits[32];
        int i = 0;

        // Extract hexadecimal digits
        while (ul != 0)
        {
            int remainder = ul % 16;
            if (remainder < 10)
                hex_digits[i++] = '0' + remainder; // Convert to ASCII
            else
                hex_digits[i++] = 'A' + (remainder - 10); // Convert to ASCII
            ul /= 16;
        }

        // Print hexadecimal digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(hex_digits[j]);
        }
    }

    void print_unsigned_long_integer(unsigned long ul)
    {
        // Handle the base case of 0
        if (ul == 0)
        {
            putc('0');
            return;
        }

        // Array to store digits
        char digits[32];
        int i = 0;

        // Extract digits
        while (ul != 0)
        {
            digits[i++] = '0' + (ul % 10); // Convert to ASCII
            ul /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(digits[j]);
        }
    }

    void print_long_long_integer(long long ll)
    {
        // Handle negative numbers
        if (ll < 0)
        {
            putc('-');
            ll = -ll;
        }

        // Handle the base case of 0
        if (ll == 0)
        {
            putc('0');
            return;
        }

        // Array to store digits
        char digits[32];
        int i = 0;

        // Extract digits
        while (ll != 0)
        {
            digits[i++] = '0' + (ll % 10); // Convert to ASCII
            ll /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(digits[j]);
        }
    }


    void print_unsigned_long_long_hexadecimal(unsigned long long ull)
    {
        // Handle the base case of 0
        if (ull == 0)
        {
            putc('0');
            return;
        }

        // Array to store hexadecimal digits
        char hex_digits[32];
        int i = 0;

        // Extract hexadecimal digits
        while (ull != 0)
        {
            int remainder = ull % 16;
            if (remainder < 10)
                hex_digits[i++] = '0' + remainder; // Convert to ASCII
            else
                hex_digits[i++] = 'A' + (remainder - 10); // Convert to ASCII
            ull /= 16;
        }

        // Print hexadecimal digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(hex_digits[j]);
        }
    }

    void print_unsigned_char_hexadecimal(unsigned char uc)
    {
        // Extract higher and lower nibbles
        unsigned char higher_nibble = uc >> 4;
        unsigned char lower_nibble = uc & 0x0F;

        // Print higher nibble
        if (higher_nibble < 10)
            putc('0' + higher_nibble);
        else
            putc('A' + (higher_nibble - 10));

        // Print lower nibble
        if (lower_nibble < 10)
            putc('0' + lower_nibble);
        else
            putc('A' + (lower_nibble - 10));
    }

    void print_unsigned_char_integer(unsigned char uc)
    {
        // Handle the base case of 0
        if (uc == 0)
        {
            putc('0');
            return;
        }

        // Array to store digits
        char digits[3]; // Maximum 3 digits for an unsigned char

        // Extract digits
        int i = 0;
        while (uc != 0)
        {
            digits[i++] = '0' + (uc % 10); // Convert to ASCII
            uc /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(digits[j]);
        }
    }

    void print_unsigned_short_hexadecimal(unsigned short us) {
        // Extract higher and lower bytes
        unsigned char higher_byte = (us >> 8) & 0xFF;
        unsigned char lower_byte = us & 0xFF;

        // Print higher byte
        unsigned char higher_nibble = higher_byte >> 4;
        unsigned char lower_nibble = higher_byte & 0x0F;

        // Print higher nibble
        if (higher_nibble < 10)
            putc('0' + higher_nibble);
        else
            putc('A' + (higher_nibble - 10));

        // Print lower nibble
        if (lower_nibble < 10)
            putc('0' + lower_nibble);
        else
            putc('A' + (lower_nibble - 10));

        // Print lower byte
        higher_nibble = lower_byte >> 4;
        lower_nibble = lower_byte & 0x0F;

        // Print higher nibble
        if (higher_nibble < 10)
            putc('0' + higher_nibble);
        else
            putc('A' + (higher_nibble - 10));

        // Print lower nibble
        if (lower_nibble < 10)
            putc('0' + lower_nibble);
        else
            putc('A' + (lower_nibble - 10));
    }

    void print_unsigned_short_integer(unsigned short usi)
    {
        // Handle the base case of 0
        if (usi == 0)
        {
            putc('0');
            return;
        }

        // Array to store digits
        char digits[5]; // Maximum 5 digits for an unsigned short

        // Extract digits
        int i = 0;
        while (usi != 0)
        {
            digits[i++] = '0' + (usi % 10); // Convert to ASCII
            usi /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(digits[j]);
        }
    }

    void print_signed_char_integer(signed char sci) {
        // Handle negative numbers
        if (sci < 0)
        {
            putc('-');
            sci = -sci;
        }

        // Handle the base case of 0
        if (sci == 0)
        {
            putc('0');
            return;
        }

        // Array to store digits
        char digits[4]; // Maximum 4 digits for a signed char

        // Extract digits
        int i = 0;
        while (sci != 0)
        {
            digits[i++] = '0' + (sci % 10); // Convert to ASCII
            sci /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(digits[j]);
        }
    }

    void print_signed_short_integer(signed short ssi) {
        // Handle negative numbers
        if (ssi < 0)
        {
            putc('-');
            ssi = -ssi;
        }

        // Handle the base case of 0
        if (ssi == 0)
        {
            putc('0');
            return;
        }

        // Array to store digits
        char digits[6]; // Maximum 6 digits for a signed short

        // Extract digits
        int i = 0;
        while (ssi != 0)
        {
            digits[i++] = '0' + (ssi % 10); // Convert to ASCII
            ssi /= 10;
        }

        // Print digits in reverse order
        for (int j = i - 1; j >= 0; j--)
        {
            putc(digits[j]);
        }
    }

    void printf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);

        unsigned int uint;

        while (*fmt)
        {
            switch (*fmt)
            {
                case '%':
                    fmt++;
                    switch (*fmt)
                    {
                        case '%':
                            putc('%');
                            fmt++;
                            break;
                        case 's':
                            puts(va_arg(args, char*));
                            fmt++;
                            break;
                        case 'c':
                            putc(static_cast<char>(va_arg(args, int)));
                            fmt++;
                            break;
                        case 'i':
                        case 'd':
                            print_signed_integer(va_arg(args, int));
                            fmt++;
                            break;
                        case 'o':
                            print_unsigned_integer_octal(va_arg(args, unsigned int));
                            fmt++;
                            break;
                        case 'x':
                            print_unsigned_integer_hexadecimal(va_arg(args, unsigned int));
                            fmt++;
                            break;
                        case 'u':
                            print_unsigned_integer(va_arg(args, unsigned int));
                            fmt++;
                            break;
                        case 'f':
                            print_double(va_arg(args, double));
                            fmt++;
                            break;
                        case 'p':
                            print_pointer(va_arg(args, void*));
                            fmt++;
                            break;
                        case 'l':
                            fmt++;
                            switch (*fmt)
                            {
                                /* %l_ */
                                case 'i':
                                case 'd':
                                    print_long(va_arg(args, long));
                                    fmt++;
                                    break;
                                case 'o':
                                    print_unsigned_long_octal(va_arg(args, unsigned long));
                                    fmt++;
                                    break;
                                case 'x':
                                    print_unsigned_long_hexadecimal(va_arg(args, unsigned long));
                                    fmt++;
                                    break;
                                case 'u':
                                    print_unsigned_long_integer(va_arg(args, unsigned long));
                                    fmt++;
                                    break;
                                case 'f':
                                    print_double(va_arg(args, double));
                                    fmt++;
                                    break;
                                case 'l':
                                    fmt++;
                                    switch (*fmt)
                                    {
                                        case 'i':
                                        case 'd':
                                            print_long_long_integer(va_arg(args, long long));
                                            fmt++;
                                            break;
                                        case 'x':
                                            print_unsigned_long_long_hexadecimal(va_arg(args, unsigned long long));
                                            fmt++;
                                            break;
                                        default:
                                            putc(*fmt);
                                            fmt++;
                                            break;
                                    }

                                    break;
                                default:
                                    putc(*fmt);
                                    fmt++;
                                    break;
                            }
                            break;
                        case 'h':
                            fmt++;
                            switch (*fmt)
                            {
                                case 'h':
                                    fmt++;
                                    switch (*fmt)
                                    {
                                        case 'x':
                                            print_unsigned_char_hexadecimal(static_cast<unsigned char>(va_arg(args, int)));
                                            fmt++;
                                            break;
                                        case 'u':
                                            print_unsigned_char_integer(static_cast<unsigned char>(va_arg(args, int)));
                                            fmt++;
                                            break;
                                        case 'i':
                                        case 'd':
                                            print_signed_char_integer(static_cast<signed char>(va_arg(args, int)));
                                            fmt++;
                                            break;
                                        default:
                                            putc(*fmt);
                                            fmt++;
                                            break;
                                    }
                                    break;
                                case 'x':
                                    print_unsigned_short_hexadecimal(static_cast<unsigned short>(va_arg(args, int)));
                                    fmt++;
                                    break;
                                case 'u':
                                    print_unsigned_short_integer(static_cast<unsigned short>(va_arg(args, int)));
                                    fmt++;
                                    break;
                                case 'i':
                                case 'd':
                                    print_signed_short_integer(static_cast<signed short>(va_arg(args, int)));
                                    fmt++;
                                    break;
                                default:
                                    putc(*fmt);
                                    fmt++;
                                    break;
                            }
                            break;
                        default:
                            putc(*fmt);
                            fmt++;
                            break;
                    }
                    break;
                default:
                    putc(*fmt);
                    fmt++;
                    break;
            }
        }

        va_end(args);
        return;
    }
}