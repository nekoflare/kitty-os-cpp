#include <cstring>
#include <cxxabi.h>
#include <debug.hpp>
#include <sys/symbols.hpp>

// Function to find a symbol by name and return its address as void*
void *find_symbol_by_name(const char *symbol_name)
{
    // Iterate over all symbols in the .symbols section
    extern const symbol_entry symbols_start;
    extern const symbol_entry symbols_end;

    for (const symbol_entry *symbol = &symbols_start; symbol < &symbols_end; ++symbol)
    {
        if (std::strcmp(symbol->symbol_name, symbol_name) == 0)
        {
            return symbol->address; // Return the address of the symbol
        }
    }

    return nullptr; // Return nullptr if the symbol was not found
}

// Debugging function to dump all exported symbols
void dump_exported_symbols()
{
    // Iterate over all symbols in the .symbols section
    extern const symbol_entry symbols_start;
    extern const symbol_entry symbols_end;

    for (const symbol_entry *symbol = &symbols_start; symbol < &symbols_end; ++symbol)
    {
        // Use debug_printf to print the symbol name and address
        debug_printf("Symbol: %s, Address: %p\n", symbol->symbol_name, symbol->address);
    }
}

EXPORT_SYMBOL(find_symbol_by_name)