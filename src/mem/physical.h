//
// Created by Neko on 20.01.2025.
//

#ifndef PHYSICAL_H
#define PHYSICAL_H

#include <limine.h>

#define PAGE_SIZE 4096

constexpr const char *memory_map_type_to_string(const uint64_t type)
{
    switch (type)
    {
    case LIMINE_MEMMAP_USABLE:
        return "Usable";
    case LIMINE_MEMMAP_RESERVED:
        return "Reserved";
    case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
        return "ACPI Reclaimable";
    case LIMINE_MEMMAP_ACPI_NVS:
        return "ACPI NVS";
    case LIMINE_MEMMAP_BAD_MEMORY:
        return "Bad Memory";
    case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
        return "Bootloader Reclaimable";
    case LIMINE_MEMMAP_KERNEL_AND_MODULES:
        return "Kernel and Modules";
    case LIMINE_MEMMAP_FRAMEBUFFER:
        return "Framebuffer";
    default:
        return "Unknown entry";
    }
}

void initialize_physical_memory();
void *allocate_physical_page();
void deallocate_physical_page(void *physical_address);
uint64_t get_highest_address();
uint64_t get_available_usable_memory();

#endif // PHYSICAL_H
