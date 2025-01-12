#include <ddk/debug.hpp>
#include <stdint.h>
#include <cstring>

constexpr uint16_t mod_maj = 1;
constexpr uint16_t mod_min = 0;
constexpr uint16_t mod_sub_ver = 0;

// Structure for managing the mixin context
struct mixin_context
{
    char original_bytecode[64];
    char mixin_bytecode[64];
    bool is_mixed_in;
    void* mixin_function_address;
    void* original_function_address;
};

// Create a mixin context
mixin_context create_mixin_context(
    void* mixin_function,
    void* original_function)
{
    mixin_context context;
    context.mixin_function_address = mixin_function;
    context.original_function_address = original_function;
    context.is_mixed_in = false;

    // Save the original function's first 64 bytes
    std::memcpy(context.original_bytecode, original_function, sizeof(context.original_bytecode));

    // Prepare mixin bytecode (example assumes x86_64 architecture)
    // mov rax, mixin_function_address
    // jmp rax
    uint8_t* bytecode = reinterpret_cast<uint8_t*>(context.mixin_bytecode);
    bytecode[0] = 0x48; // mov rax, immediate (64-bit)
    bytecode[1] = 0xB8;
    *reinterpret_cast<void**>(&bytecode[2]) = mixin_function; // Address of mixin_function
    bytecode[10] = 0xFF; // jmp rax
    bytecode[11] = 0xE0;

    // Fill remaining bytes with NOPs (optional for alignment)
    std::memset(&bytecode[12], 0x90, sizeof(context.mixin_bytecode) - 12);

    return context;
}

// Apply the mixin function (hook)
void mix_it_in(mixin_context& context)
{
    if (context.is_mixed_in)
        return;

    // Overwrite the original function with the mixin bytecode
    std::memcpy(context.original_function_address, context.mixin_bytecode, sizeof(context.mixin_bytecode));
    context.is_mixed_in = true;
}

// Remove the mixin and restore the original function
void unmix_it_off(mixin_context& context)
{
    if (!context.is_mixed_in)
        return;

    // Restore the original function bytecode
    std::memcpy(context.original_function_address, context.original_bytecode, sizeof(context.original_bytecode));
    context.is_mixed_in = false;
}

// Original function (to be hooked)
void og_function()
{
    debug_printf("Hey! Orig\n");
}

// Hook function (replaces the original function)
void hook_function()
{
    debug_printf("Hey! Hooked!\n");
}

// Module main for testing
void module_main()
{
    debug_printf("Module version %d.%d.%d\n", mod_maj, mod_min, mod_sub_ver);
}
