#include <debug.hpp>
#include <mem/common_memory.hpp>
#include <mem/physical_memory.hpp>
#include <stddef.h>
#include <sys/mutex.hpp>
#include <sys/symbols.hpp>

constexpr const char *type_to_string(uint64_t type)
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

// A structure to hold freelist for O(1) allocations.
struct freelist_entry
{
    freelist_entry *next;
    size_t size;
};

freelist_entry *head = nullptr;

static void push_to_freelist(freelist_entry *entry)
{
    if (head == nullptr)
    {
        head = entry;
        return;
    }

    entry->next = head;
    head = entry;
}

static freelist_entry *pop_from_freelist()
{
    if (head == nullptr)
    {
        return nullptr;
    }

    freelist_entry *entry = head;
    head = head->next;
    return entry;
}

memory_statistics mem_stats;

memory_statistics *get_memory_statistics()
{
    return &mem_stats;
}

void initialize_phys_memory(void)
{
    auto memory_map = get_memory_map();
    if (memory_map == nullptr)
    {
        debug_printf("No memory map has been found.\n");
        asm volatile("cli; hlt");
    }

    // Tag usable memory using hhdm.
    for (uint64_t i = 0; memory_map->entry_count > i; i++)
    {
        auto entry = memory_map->entries[i];
        debug_printf("%3lu. %-22s %016lX -> %016lX\n", i + 1, type_to_string(entry->type), entry->base,
                     entry->base + entry->length);

        mem_stats.overall += entry->length;
        switch (entry->type)
        {
        case LIMINE_MEMMAP_USABLE:
            mem_stats.usable += entry->length;
            break;
        case LIMINE_MEMMAP_RESERVED:
            mem_stats.reserved += entry->length;
            break;
        case LIMINE_MEMMAP_BAD_MEMORY:
            mem_stats.bad += entry->length;
            break;
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            mem_stats.kernel += entry->length;
            break;
        case LIMINE_MEMMAP_FRAMEBUFFER:
            mem_stats.other += entry->length;
            break;
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            mem_stats.reclaimable += entry->length;
            break;
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            mem_stats.reclaimable += entry->length;
            break;
        default:
            break;
        }

        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            freelist_entry *freelist_entry = reinterpret_cast<struct freelist_entry *>(
                reinterpret_cast<uintptr_t>(entry->base) + get_higher_half_offset());
            freelist_entry->next = nullptr;
            freelist_entry->size = entry->length;
            push_to_freelist(freelist_entry);
        }
    }
}

Mutex allocate_mutex;
void *allocate_page(void)
{
    allocate_mutex.lock();
    freelist_entry *entry = pop_from_freelist();
    if (entry == nullptr)
    {
        debug_printf("No available pages to allocate.\n");
        allocate_mutex.unlock();
        return nullptr;
    }

    if (entry->size > 4096)
    {
        freelist_entry *new_entry = reinterpret_cast<freelist_entry *>(reinterpret_cast<uintptr_t>(entry) + 4096);
        new_entry->size = entry->size - 4096;
        new_entry->next = entry->next;
        entry->next = nullptr;
        entry->size = 4096;

        push_to_freelist(new_entry);
    }

    uintptr_t base_address = reinterpret_cast<uintptr_t>(entry);
    mem_stats.usable -= 4096;
    allocate_mutex.unlock();
    return reinterpret_cast<void *>(base_address - get_higher_half_offset());
}

void deallocate_page(void *page)
{
    allocate_mutex.lock();
    uintptr_t page_address = reinterpret_cast<uintptr_t>(page);
    freelist_entry *entry = reinterpret_cast<freelist_entry *>(page_address + get_higher_half_offset());
    mem_stats.usable += 4096;

    push_to_freelist(entry);
    allocate_mutex.unlock();
}

EXPORT_SYMBOL(initialize_phys_memory);
EXPORT_SYMBOL(allocate_page);
EXPORT_SYMBOL(deallocate_page);
EXPORT_SYMBOL(get_memory_statistics);
