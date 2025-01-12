#pragma once

struct symbol_entry
{
    const char *symbol_name;
    void *address;
};

#define EXPORT_SYMBOL(func)                                                                                            \
    __attribute__((section(".symbols"), used)) static const struct symbol_entry __symbol_##func = {#func,              \
                                                                                                   (void *)&func};

void call_hooks(const char *symbol_name);
void *find_symbol_by_name(const char *symbol_name);
void dump_exported_symbols();