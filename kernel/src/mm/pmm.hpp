//
// Created by Piotr on 17.05.2024.
//

#ifndef KITTY_OS_CPP_PMM_HPP
#define KITTY_OS_CPP_PMM_HPP

#include <mm/internal/memory_bitmap.hpp>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <limine.h>
#include <type_traits>
#include <kstd/kstdio.hpp>
#include <mm/vmm.hpp>
#include <arch/x64/control/control.hpp>
#include <kstd/kbitmap.hpp>
#include <kstd/kstring.hpp>

/*
 * Structs
 */
typedef struct {
    double size;
    const char* prefix;
} mem_size;

/*
 * Raw defines
 */
#define PAGE_SIZE 4096

/*
 * constexpr functions
 */
template <typename T>
constexpr size_t calculate_page_index(T address)
{
    if constexpr (std::is_same_v<T, uint64_t>)
    {
        size_t page_index = address / PAGE_SIZE;

        if (address % PAGE_SIZE != 0) --page_index;

        return page_index;
    }

    size_t page_index = reinterpret_cast<uint64_t>(address) / PAGE_SIZE;
    if (reinterpret_cast<uint64_t>(address) % PAGE_SIZE != 0) --page_index;
    return page_index;
}

constexpr const char* pmm_limine_memmap_type_to_string(uint64_t memmap_type)
{
    switch (memmap_type)
    {
        case LIMINE_MEMMAP_USABLE:
            return "Usable memory";
        case LIMINE_MEMMAP_RESERVED:
            return "Reserved memory";
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            return "ACPI reclaimable memory";
        case LIMINE_MEMMAP_ACPI_NVS:
            return "ACPI Non-Volatile Storage";
        case LIMINE_MEMMAP_BAD_MEMORY:
            return "Bad memory";
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            return "Bootloader reclaimable memory";
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            return "Kernel and modules (Reserved)";
        case LIMINE_MEMMAP_FRAMEBUFFER:
            return "Framebuffer memory";
        default:
            return "Invalid memory map entry type.";
    }
}

/*
 * static functions
 */
mem_size pmm_calculate_effective_size(uint64_t size);

/*
 * Global function definitions
 */

void pmm_init();
void pmm_print_limine_memmap_entries();
void pmm_print_memory_usage();
void pmm_print_memory_information();
void pmm_print_unaligned_memory_map_entries();

uint64_t pmm_alloc_page();
void pmm_free_page(uint64_t addr);

/*
 * Classes
 */
class PMMBitmap : public Bitmap<uint8_t>
{
public:
    void mark_page(uint64_t page_index);
    void mark_addr(uint64_t address);
    void mark_pages_used_in_range(uint64_t page_index, size_t len);
    void mark_addrs_used_in_range(uint64_t address, size_t len);
    void unmark_page(uint64_t page_index);
    void unmark_addr(uint64_t address);
    void unmark_pages_in_range(uint64_t page_index, size_t len);
    void unmark_addrs_in_range(uint64_t addr, size_t len);
private:
};

#endif //KITTY_OS_CPP_PMM_HPP
