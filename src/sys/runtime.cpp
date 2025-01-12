#include <cmath>
#include <debug.hpp>
#include <locale.h>
#include <stddef.h>
#include <sys/nanoprintf.hpp>
#include <sys/runtime.hpp>
#include <sys/symbols.hpp>
#include <xxHash/xxhash.h>

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
    // debug_printf("Called __errno_location()\n");
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
    debug_printf("__cxa_atexit(f=%p obj=%p dso=%p) has been called.\n", f, obj, dso);
    // asm volatile ("cli; hlt");

    return 0;
}

// exceptions
namespace std
{
void __throw_bad_alloc()
{
    debug_printf("__throw_bad_alloc(): Has been called. Halting.\n");
    asm volatile("cli; hlt");
}

void __throw_bad_array_new_length()
{
    debug_printf("__throw_bad_array_new_length(): Has been called. Halting.\n");
    asm volatile("cli; hlt");
}

void __throw_length_error(char const *msg)
{
    debug_printf("__throw_length_error(): %s. Halting.\n", msg);
    asm volatile("cli; hlt");
}

void __throw_bad_optional_access()
{
    debug_printf("__throw_bad_optional_access(): Has been called. Halting.\n");
    asm volatile("cli; hlt");
}

void __throw_bad_function_call()
{
    debug_printf("__throw_bad_function_call(): Has been called. Halting.\n");
    asm volatile("cli; hlt");
}

void __throw_out_of_range_fmt(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
    debug_printf("__throw_out_of_range_fmt(): %s. Halting.\n", buffer);
    va_end(args);
    asm volatile("cli; hlt");
}

void __throw_logic_error(char const *msg)
{
    debug_printf("__throw_logic_error(): %s. Halting\n", msg);
    asm volatile("cli; hlt");
}

void __throw_out_of_range(char const *msg)
{
    debug_printf("__throw_out_of_range(): %s. Halting\n", msg);
    asm volatile("cli; hlt");
}

void __throw_invalid_argument(char const *msg)
{
    debug_printf("__throw_invalid_argument(): %s. Halting\n", msg);
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
        debug_printf("*** Stack smashing detected ***\n");
        while (true)
        {
            asm volatile("cli; hlt");
        }
    }
}

extern "C" void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *function)
{
    debug_printf("__assert_fail(): %s at: %s:%d in function: %s\n", assertion, file, line, function);
    asm volatile("cli; hlt");
}

extern "C" void abort()
{
    debug_printf("Aborting!\n");
    asm volatile("cli; hlt");
}

extern "C" int snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // Use from nanoprintf library to format the string into the buffer
    int written = npf_vsnprintf(str, size, format, args);

    va_end(args);

    // Return the number of characters that would have been written (excluding null terminator)
    return written;
}