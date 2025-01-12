#include <climits>
#include <cstdlib>
#include <stddef.h>
#include <sys/symbols.hpp>

extern "C"
{
    void *memset(void *s, int c, size_t n)
    {
        unsigned char *p = static_cast<unsigned char *>(s);
        while (n--)
        {
            *p++ = static_cast<unsigned char>(c);
        }
        return s;
    }

    int memcmp(const void *s1, const void *s2, size_t n)
    {
        const unsigned char *p1 = static_cast<const unsigned char *>(s1);
        const unsigned char *p2 = static_cast<const unsigned char *>(s2);

        for (size_t i = 0; i < n; ++i)
        {
            if (p1[i] < p2[i])
            {
                return -1;
            }
            else if (p1[i] > p2[i])
            {
                return 1;
            }
        }

        return 0;
    }

    int strcmp(const char *str1, const char *str2)
    {
        while (*str1 != '\0' && *str2 != '\0')
        {
            if (*str1 != *str2)
            {
                return static_cast<unsigned char>(*str1) - static_cast<unsigned char>(*str2);
            }
            str1++;
            str2++;
        }

        return static_cast<unsigned char>(*str1) - static_cast<unsigned char>(*str2);
    }

    void *memcpy(void *dest, const void *src, size_t n)
    {
        unsigned char *d = static_cast<unsigned char *>(dest);
        const unsigned char *s = static_cast<const unsigned char *>(src);

        while (n--)
        {
            *d++ = *s++;
        }

        return dest;
    }

    void *memmove(void *dest, const void *src, size_t n)
    {
        unsigned char *d = static_cast<unsigned char *>(dest);
        const unsigned char *s = static_cast<const unsigned char *>(src);

        if (d < s || d >= (s + n))
        {
            // No overlap, or destination is before the source. Copy forwards
            while (n--)
            {
                *d++ = *s++;
            }
        }
        else
        {
            // Overlap, copy backwards
            d += n;
            s += n;
            while (n--)
            {
                *(--d) = *(--s);
            }
        }

        return dest;
    }

    size_t strlen(const char *str)
    {
        size_t length = 0;
        while (str[length] != '\0')
        {
            ++length;
        }
        return length;
    }

    const void *memchr(const void *__s, int __c, size_t __n)
    {
        const unsigned char *p = static_cast<const unsigned char *>(__s);
        unsigned char target = static_cast<unsigned char>(__c);

        for (size_t i = 0; i < __n; ++i)
        {
            if (p[i] == target)
            {
                return const_cast<void *>(static_cast<const void *>(&p[i]));
            }
        }
        return nullptr;
    }

    long strtol(const char *nptr, char **endptr, int base)
    {
        const char *p = nptr;
        while (*p == ' ' || *p == '\t') // Skip whitespace
            ++p;

        bool negative = false;
        if (*p == '-')
        {
            negative = true;
            ++p;
        }
        else if (*p == '+')
        {
            ++p;
        }

        long result = 0;
        while (*p)
        {
            int digit = -1;
            if (*p >= '0' && *p <= '9')
                digit = *p - '0';
            else if (*p >= 'A' && *p <= 'Z')
                digit = *p - 'A' + 10;
            else if (*p >= 'a' && *p <= 'z')
                digit = *p - 'a' + 10;

            if (digit < 0 || digit >= base)
                break;

            result = result * base + digit;
            ++p;
        }

        if (endptr)
            *endptr = const_cast<char *>(p);

        return negative ? -result : result;
    }

    float strtof(const char *nptr, char **endptr)
    {
        const char *p = nptr;
        while (*p == ' ' || *p == '\t') // Skip whitespace
            ++p;

        bool negative = false;
        if (*p == '-')
        {
            negative = true;
            ++p;
        }
        else if (*p == '+')
        {
            ++p;
        }

        float result = 0.0f;
        while (*p >= '0' && *p <= '9') // Integer part
        {
            result = result * 10.0f + (*p - '0');
            ++p;
        }

        if (*p == '.') // Fractional part
        {
            ++p;
            float fraction = 0.1f;
            while (*p >= '0' && *p <= '9')
            {
                result += (*p - '0') * fraction;
                fraction *= 0.1f;
                ++p;
            }
        }

        if ((*p == 'e' || *p == 'E') && (*(p + 1) == '+' || *(p + 1) == '-' || (*(p + 1) >= '0' && *(p + 1) <= '9')))
        {
            ++p;
            bool exp_negative = false;
            if (*p == '-')
            {
                exp_negative = true;
                ++p;
            }
            else if (*p == '+')
            {
                ++p;
            }

            int exponent = 0;
            while (*p >= '0' && *p <= '9')
            {
                exponent = exponent * 10 + (*p - '0');
                ++p;
            }

            float exp_factor = 1.0f;
            while (exponent--)
                exp_factor *= 10.0f;

            result *= exp_negative ? (1.0f / exp_factor) : exp_factor;
        }

        if (endptr)
            *endptr = const_cast<char *>(p);

        return negative ? -result : result;
    }

    int toupper(int c)
    {
        if (c >= 'a' && c <= 'z')
            return c - ('a' - 'A');
        return c;
    }

    int tolower(int c)
    {
        if (c >= 'A' && c <= 'Z')
            return c + ('a' - 'A');
        return c;
    }

    long long int strtoll(const char *str, char **endptr, int base)
    {
        if (str == nullptr)
        {
            if (endptr)
                *endptr = const_cast<char *>(str);
            return 0;
        }

        // Skip leading white spaces
        while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' || *str == '\f' || *str == '\v')
        {
            str++;
        }

        // Check for sign
        bool negative = false;
        if (*str == '-')
        {
            negative = true;
            str++;
        }
        else if (*str == '+')
        {
            str++;
        }

        // If the base is 0, detect the base from the input string
        if (base == 0)
        {
            if (*str == '0')
            {
                str++;
                if (*str == 'x' || *str == 'X')
                {
                    base = 16; // Hexadecimal
                    str++;
                }
                else
                {
                    base = 8; // Octal
                }
            }
            else
            {
                base = 10; // Decimal by default
            }
        }

        long long int result = 0;
        bool overflow = false;

        while (true)
        {
            char digit = *str;

            // Handle hexadecimal digits if base is 16
            if (base == 16 &&
                ((digit >= '0' && digit <= '9') || (digit >= 'a' && digit <= 'f') || (digit >= 'A' && digit <= 'F')))
            {
                if (digit >= '0' && digit <= '9')
                {
                    digit -= '0';
                }
                else if (digit >= 'a' && digit <= 'f')
                {
                    digit -= 'a' - 10;
                }
                else
                {
                    digit -= 'A' - 10;
                }
            }
            // Handle octal digits if base is 8
            else if (base == 8 && (digit >= '0' && digit <= '7'))
            {
                digit -= '0';
            }
            // Handle decimal digits if base is 10
            else if (base == 10 && (digit >= '0' && digit <= '9'))
            {
                digit -= '0';
            }
            else
            {
                break; // Invalid character
            }

            // Handle overflow detection
            if (result > (LLONG_MAX - digit) / base)
            {
                overflow = true;
                break;
            }

            result = result * base + digit;
            str++;
        }

        // If negative, negate the result
        if (negative)
        {
            result = -result;
        }

        // Set endptr if it's not null
        if (endptr)
        {
            *endptr = const_cast<char *>(str);
        }

        // Handle overflow and underflow cases
        if (overflow)
        {
            if (negative)
            {
                return LLONG_MIN;
            }
            else
            {
                return LLONG_MAX;
            }
        }

        return result;
    }

    char *strdup(const char *str)
    {

        // Allocate memory to store the duplicate string
        size_t len = strlen(str) + 1; // +1 for the null terminator
        char *copy = static_cast<char *>(malloc(len));

        if (copy == nullptr)
        {
            return nullptr; // Memory allocation failed
        }

        // Copy the string into the allocated memory
        memcpy(copy, str, len);

        return copy;
    }
}

EXPORT_SYMBOL(memset);
EXPORT_SYMBOL(memcmp);
EXPORT_SYMBOL(strcmp);
EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memmove);
EXPORT_SYMBOL(strlen);
EXPORT_SYMBOL(memchr);
EXPORT_SYMBOL(strtol);
EXPORT_SYMBOL(strtof);
EXPORT_SYMBOL(toupper);
EXPORT_SYMBOL(tolower);