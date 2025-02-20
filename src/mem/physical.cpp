//
// Created by Neko on 20.01.2025.
//

#include "physical.h"

#include <cstddef>
#include <dbg/log.h>

#include "virtual.h"

limine_memmap_request memory_map_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = nullptr,
};

limine_memmap_entry **get_memory_map()
{
    if (!memory_map_request.response)
    {
        return nullptr;
    }

    return memory_map_request.response->entries;
}

uint64_t get_memory_map_entry_count()
{
    if (!memory_map_request.response)
    {
        return 0;
    }

    return memory_map_request.response->entry_count;
}

struct phys_freelist_entry
{
    phys_freelist_entry *next{};
    size_t size{};
};

phys_freelist_entry *head = nullptr;

[[maybe_unused]] static void push_to_freelist(phys_freelist_entry *entry)
{
    if (head == nullptr)
    {
        head = entry;

        return;
    }

    entry->next = head;
    head = entry;
}

[[maybe_unused]] static phys_freelist_entry *pop_from_freelist()
{
    if (head == nullptr)
    {
        return nullptr;
    }

    phys_freelist_entry *entry = head;
    head = head->next;
    return entry;
}

void initialize_physical_memory()
{
    auto memory_map = get_memory_map();
    auto entry_count = get_memory_map_entry_count();

    if (entry_count == 0 || memory_map == nullptr)
    {
        debug_print("No memory.\n");
        asm volatile("cli; hlt");
        __builtin_unreachable();
    }

    for (size_t i = 0; i < entry_count; i++)
    {
        auto entry = memory_map[i];

        debug_print("%3lu. %-22s %016lX -> %016lX\n", i + 1, memory_map_type_to_string(entry->type), entry->base,
                    entry->base + entry->length);

        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            auto *freelist_entry =
                reinterpret_cast<struct phys_freelist_entry *>(entry->base + get_higher_half_memory_offset());
            freelist_entry->next = nullptr;
            freelist_entry->size = entry->length;
            push_to_freelist(freelist_entry);
        }
    }
}

void *allocate_physical_page()
{
    phys_freelist_entry *entry = pop_from_freelist();
    if (entry == nullptr)
    {
        debug_print("No available pages to allocate.\n");
        return nullptr;
    }

    if (entry->size > 4096)
    {
        phys_freelist_entry *new_entry =
            reinterpret_cast<phys_freelist_entry *>(reinterpret_cast<uintptr_t>(entry) + 4096);
        new_entry->size = entry->size - 4096;
        new_entry->next = entry->next;
        entry->next = nullptr;
        entry->size = 4096;

        push_to_freelist(new_entry);
    }

    auto base_address = reinterpret_cast<uintptr_t>(entry);
    return reinterpret_cast<void *>(base_address - get_higher_half_memory_offset());
}

void deallocate_physical_page(void *page)
{
    auto page_address = reinterpret_cast<uintptr_t>(page);
    auto *entry = reinterpret_cast<phys_freelist_entry *>(page_address + get_higher_half_memory_offset());

    push_to_freelist(entry);
}

uint64_t get_highest_address()
{
    const auto entries = get_memory_map();
    const auto entry_count = get_memory_map_entry_count();

    if (entry_count == 0 || entries == nullptr)
        return 0;

    uint64_t highest_address = 0;

    for (size_t i = 0; i < entry_count; i++)
    {
        if (const auto entry = entries[i]; entry->base + entry->length > highest_address)
        {
            highest_address = entry->base + entry->length;
        }
    }

    return highest_address;
}

uint64_t get_available_usable_memory()
{
    uint64_t available_memory = 0;
    auto entry = head;
    while (entry)
    {
        available_memory += entry->size;
        entry = entry->next;
    }
    return available_memory;
}