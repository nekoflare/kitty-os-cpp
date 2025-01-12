#pragma once

#include <stdint.h>

struct memory_statistics
{
    uint64_t overall;
    uint64_t usable;
    uint64_t reserved;
    uint64_t bad;
    uint64_t kernel;      // kernel + bootloader
    uint64_t other;       // framebuffer + acpi
    uint64_t reclaimable; // acpi reclailable + bootloader reclaimable
};

void initialize_phys_memory(void);
void *allocate_page(void);
void deallocate_page(void *page);
memory_statistics *get_memory_statistics();
