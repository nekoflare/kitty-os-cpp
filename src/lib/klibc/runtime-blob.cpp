//
// Created by Neko on 20.01.2025.
//

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <dbg/log.h>
#include <lib/npf/nanoprintf.h>
#include <lib/xxHash/xxhash.h>
#include <limits.h>
#include <locale.h>

extern void (*__init_array[])();
extern void (*__init_array_end[])();

int errno = 0;

__attribute__((weak)) bool __libc_single_threaded = false;

// Default locale information for the "C" locale (basic/default locale)
static struct lconv default_lconv = {
    ".", // decimal_point
    "",  // thousands_sep
    "",  // grouping
    "",  // int_curr_symbol
    "",  // currency_symbol
    ".", // mon_decimal_point
    "",  // mon_thousands_sep
    "",  // mon_grouping
    "",  // positive_sign
    "-", // negative_sign
    127, // int_frac_digits (CHAR_MAX)
    127, // frac_digits (CHAR_MAX)
    1,   // p_cs_precedes (positive)
    0,   // p_sep_by_space (positive)
    1,   // n_cs_precedes (negative)
    0,   // n_sep_by_space (negative)
    1,   // p_sign_posn (positive)
    1    // n_sign_posn (negative)
};

// Function definition of localeconv
struct lconv *localeconv(void)
{
    return &default_lconv; // Return the static locale information
}

extern "C" int *__errno_location(void)
{
    // debug_print("Called __errno_location()\n");
    return &errno;
}

void initialize_runtime()
{
    for (size_t i = 0; &__init_array[i] != __init_array_end; i++)
    {
        __init_array[i]();
    }
}

// Cxx shit.

// whatever dso is
extern "C" void *__dso_handle;
void *__dso_handle = nullptr;

extern "C" int __cxa_atexit(void (*f)(void *), void *obj, void *dso)
{
    debug_print("__cxa_atexit(f=%p obj=%p dso=%p) has been called.\n", f, obj, dso);
    // asm volatile ("cli; hlt");

    return 0;
}

// exceptions
namespace std
{
void __throw_bad_alloc()
{
    debug_print("__throw_bad_alloc(): Has been called. Halting.\n");
    asm volatile("cli; hlt");
}

void __throw_bad_array_new_length()
{
    debug_print("__throw_bad_array_new_length(): Has been called. Halting.\n");
    asm volatile("cli; hlt");
}

void __throw_length_error(char const *msg)
{
    debug_print("__throw_length_error(): %s. Halting.\n", msg);
    asm volatile("cli; hlt");
}

void __throw_bad_optional_access()
{
    debug_print("__throw_bad_optional_access(): Has been called. Halting.\n");
    asm volatile("cli; hlt");
}

void __throw_bad_function_call()
{
    debug_print("__throw_bad_function_call(): Has been called. Halting.\n");
    asm volatile("cli; hlt");
}

void __throw_out_of_range_fmt(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
    debug_print("__throw_out_of_range_fmt(): %s. Halting.\n", buffer);
    va_end(args);
    asm volatile("cli; hlt");
}

void __throw_logic_error(char const *msg)
{
    debug_print("__throw_logic_error(): %s. Halting\n", msg);
    asm volatile("cli; hlt");
}

void __throw_out_of_range(char const *msg)
{
    debug_print("__throw_out_of_range(): %s. Halting\n", msg);
    asm volatile("cli; hlt");
}

void __throw_invalid_argument(char const *msg)
{
    debug_print("__throw_invalid_argument(): %s. Halting\n", msg);
    asm volatile("cli; hlt");
}

} // namespace std

// STL
namespace std
{
std::size_t _Hash_bytes(void const *bytes, unsigned long len, unsigned long seed)
{
    return XXH64(bytes, len, seed);
}

namespace __detail
{
    class _Prime_rehash_policy
    {
      public:
        [[gnu::used]] bool _M_need_rehash(unsigned long current_size, unsigned long num_elements,
                                          unsigned long max_load_factor) const
        {
            double load_factor = static_cast<double>(num_elements) / current_size;

            return load_factor > max_load_factor;
        }

        [[gnu::used]] unsigned long _M_next_bkt(unsigned long current_bkt) const
        {
            return current_bkt + 1; // Incrementing to the next bucket
        }
    };
} // namespace __detail
} // namespace std

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

[[gnu::used]] uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

extern "C"
{

    __attribute__((noreturn)) void __stack_chk_fail(void)
    {
        debug_print("*** Stack smashing detected ***\n");
        while (true)
        {
            asm volatile("cli; hlt");
        }
    }
}

extern "C" void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function)
{
    debug_print("__assert_fail(): %s at: %s:%d in function: %s\n", assertion, file, line, function);
    asm volatile("cli; hlt");
}

extern "C" void abort()
{
    debug_print("Aborting!\n");
    asm volatile("cli; hlt");
}

extern "C" int snprintf(char *str, const size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // Use from nanoprintf library to format the string into the buffer
    const int written = npf_vsnprintf(str, size, format, args);

    va_end(args);

    // Return the number of characters that would have been written (excluding
    // null terminator)
    return written;
}

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

    void *__memcpy_chk(void *dest, const void *src, size_t len, size_t dest_len)
    {
        if (len > dest_len)
        {
            debug_print("Warning! __memcpy_chk: len > dest_len\n");
            asm volatile("cli; hlt");
            __builtin_unreachable();
        }

        // Perform the actual memory copy
        unsigned char *d = (unsigned char *)dest;
        const unsigned char *s = (const unsigned char *)src;

        for (size_t i = 0; i < len; i++)
        {
            d[i] = s[i];
        }

        return dest;
    }

    int __fprintf_chk(void *stream, int flag, const char *format, size_t dest_len, ...)
    {
        // Check if the stream is valid
        if (stream == NULL || format == NULL)
        {
            return -1; // Return error code for invalid input
        }

        // The actual buffer for the formatted output
        char buffer[1024]; // You may need to dynamically allocate this depending on
                           // your system

        // Variable argument list initialization
        va_list args;
        va_start(args, format);

        // Perform the formatted output into the buffer
        int len = npf_vsnprintf(buffer, sizeof(buffer), format, args);

        // End the variable argument list processing
        va_end(args);

        // Check if the destination buffer can hold the formatted output
        if (len >= dest_len)
        {
            // If the formatted string is larger than the destination buffer, report an
            // overflow
            return -1; // Return error code for buffer overflow
        }

        // Write the formatted string to the provided stream
        debug_print("%s", buffer);
        return 1;
    }

    int strncmp(const char *str1, const char *str2, size_t n)
    {
        for (size_t i = 0; i < n; i++)
        {
            if (str1[i] == '\0' && str2[i] == '\0')
            {
                return 0;
            }
            if (str1[i] != str2[i])
            {
                return (unsigned char)str1[i] - (unsigned char)str2[i];
            }
        }
        return 0;
    }

    double pow(double x, double y)
    {
        if (y == 0)
        {
            return 1.0; // x^0 is always 1
        }

        double result = 1.0;
        if (y > 0)
        {
            for (int i = 0; i < y; i++)
            {
                result *= x;
            }
        }
        else
        {
            for (int i = 0; i < -y; i++)
            {
                result /= x;
            }
        }

        return result;
    }

    int abs(int x)
    {
        return (x < 0) ? -x : x;
    }

    float powf(const float x, const float y)
    {
        if (y == 0)
        {
            return 1.0f; // x^0 is always 1
        }

        float result = 1.0f;
        if (y > 0)
        {
            for (int i = 0; i < static_cast<int>(y); i++)
            {
                result *= x;
            }
        }
        else
        {
            for (int i = 0; i < static_cast<int>(-y); i++)
            {
                result /= x;
            }
        }

        return result;
    }

    float ldexpf(const float x, const int exp)
    {
        return x * (1 << exp);
    }

    typedef struct
    {
        volatile int locked;
    } mutex_t;

    // Atomic Test-and-Set
    static inline int atomic_test_and_set(volatile int *ptr)
    {
        int old = 1;
        __asm__ __volatile__("xchg %0, %1" : "=r"(old), "=m"(*ptr) : "0"(old), "m"(*ptr) : "memory");
        return old;
    }

    // Mutex Lock (Spinlock)
    void pthread_mutex_lock(mutex_t *mutex)
    {
        while (atomic_test_and_set(&mutex->locked))
        {
            // Busy wait (spin)
            while (mutex->locked)
                ;
        }
    }

    // Mutex Unlock
    void pthread_mutex_unlock(mutex_t *mutex)
    {
        __asm__ __volatile__("mfence" ::: "memory"); // Memory barrier
        mutex->locked = 0;
    }

    // Custom mprotect using syscall
    int mprotect(void *addr, size_t len, int prot)
    {
        // lol
        debug_print("mprotect(%p, %zu, %d) = 0\n", addr, len, prot);
    }

    int fprintf(FILE *stream, const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        // Forward the call to npf_fprintf
        char buffer[4096]{0};
        const int result = npf_vsnprintf(buffer, 4096, format, args);
        debug_print("%s", buffer);

        va_end(args);
        return result;
    }

    int getpagesize()
    {
        return 4096;
    }
}

__attribute__((used)) _IO_FILE *stderr = nullptr;