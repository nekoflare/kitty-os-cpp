#pragma once

#include <stddef.h>

struct kernel_module
{
    void *address;
    size_t size;
};

void print_modules();
kernel_module get_module(const char *path);