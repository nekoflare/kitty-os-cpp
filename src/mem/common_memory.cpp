#include <limine.h>
#include <mem/common_memory.hpp>
#include <stdint.h>
#include <sys/symbols.hpp>

volatile limine_hhdm_request hhdm = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = nullptr,
};

volatile limine_memmap_request memory_map = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = nullptr,
};

volatile limine_kernel_address_request kernel_address = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST, .revision = 0, .response = nullptr};

void* kernel_page_map = nullptr;

void set_kernel_page_map()
{
    kernel_page_map = get_pml4();
}

void* get_kernel_page_map()
{
    return reinterpret_cast<void*>(get_pml4());
}

uint64_t get_kernel_base_address()
{
    return kernel_address.response->virtual_base;
}

uint64_t get_highest_memory_map_address()
{
    auto memory_map = get_memory_map();
    uint64_t greatest_address = 0;
    for (uint64_t i = 0; memory_map->entry_count > i; i++)
    {
        auto entry = memory_map->entries[i];
        if (entry->base + entry->length > greatest_address)
            greatest_address = entry->base + entry->length;
    }
    return greatest_address;
}

uintptr_t get_higher_half_offset()
{
    // we're assuming that response isn't nullptr.
    return hhdm.response->offset;
}

limine_memmap_response *get_memory_map()
{
    if (memory_map.response == nullptr)
    {
        return nullptr;
    }

    return memory_map.response;
}

EXPORT_SYMBOL(get_higher_half_offset);
EXPORT_SYMBOL(get_kernel_base_address);
EXPORT_SYMBOL(get_highest_memory_map_address);
EXPORT_SYMBOL(get_memory_map);