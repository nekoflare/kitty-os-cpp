//
// Created by Piotr on 17.05.2024.
//

#include "pmm.hpp"

/*
 * Variable initialization.
 */
static size_t pmm_limine_memmap_entry_count = 0;
static limine_memmap_entry** pmm_limine_memmap_entries = nullptr;
static limine_memmap_response* pmm_limine_memmap_response = nullptr;
limine_memmap_request pmm_limine_memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST,
        .revision = 0,
        .response = nullptr
};
static size_t pmm_overall_memory = 0;
static size_t pmm_usable_memory = 0;
static size_t pmm_unusable_memory = 0;
static size_t pmm_reserved_memory = 0;
static size_t pmm_bad_memory = 0;
static uint8_t* pmm_memory_bitmap_raw = nullptr;
static size_t pmm_memory_bitmap_size = 0;

constexpr bool pmm_verbose = true;
static bool memory_calc_lock = false;

static PMMBitmap pmm_bitmap_controller;

static void pmm_lock_memory_recalculation()
{
    memory_calc_lock = true;
}

static void pmm_calculate_memory(limine_memmap_entry** entries, const size_t entry_count)
{
    if (memory_calc_lock)
    {
        if constexpr(pmm_verbose)
        {
            kstd::printf("[PMM] Attempt of calling locked memory calculation method.\n");
        }

        return;
    }

    if constexpr (pmm_verbose)
    {
        kstd::printf("[PMM] Recalculating memory.\n");
    }

    pmm_usable_memory = 0;
    pmm_unusable_memory = 0;
    pmm_bad_memory = 0;
    pmm_reserved_memory = 0;
    pmm_overall_memory = 0;

    for (size_t i = 0; i < entry_count; i++)
    {
        limine_memmap_entry* entry = entries[i];

        pmm_overall_memory += entry->length;

        switch (entry->type)
        {
            case LIMINE_MEMMAP_USABLE:
                pmm_usable_memory += entry->length;
                break;
            case LIMINE_MEMMAP_RESERVED:
                pmm_unusable_memory += entry->length;
                pmm_reserved_memory += entry->length;
                break;
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            case LIMINE_MEMMAP_FRAMEBUFFER:
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            case LIMINE_MEMMAP_ACPI_NVS:
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                pmm_reserved_memory += entry->length;
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                pmm_bad_memory += entry->length;
                pmm_unusable_memory += entry->length;
                break;
            default:
                if constexpr (pmm_verbose)
                {
                    kstd::printf("[PMM] There's an invalid memory map entry at index: %lx and has numeric type number of: %ld.\n", i, entry->type);
                }
                break;
        }
    }
}

static void pmm_print_memmap_entry(const size_t i, const limine_memmap_entry* entry)
{
    mem_size s = pmm_calculate_effective_size(entry->length);
    kstd::printf("%ld. %lx -> %lx (%s) (%f %sB)\n", i, entry->base, entry->base + entry->length,
                 pmm_limine_memmap_type_to_string(entry->type), s.size, s.prefix);
}

void pmm_print_memory_information()
{
    pmm_print_limine_memmap_entries();
    pmm_print_memory_usage();
    pmm_print_unaligned_memory_map_entries();
}

void pmm_print_limine_memmap_entries()
{
    if (pmm_limine_memmap_entries == nullptr)
        return;

    kstd::printf("[PMM] Printing memory map entries: \n");

    for (size_t i = 0; i < pmm_limine_memmap_entry_count; i++)
    {
        const limine_memmap_entry* entry = pmm_limine_memmap_entries[i];

        kstd::printf("    "); // Add padding
        pmm_print_memmap_entry(i, entry);
    }
}

mem_size pmm_calculate_effective_size(uint64_t size)
{
    double new_size = static_cast<double>(size);
    if (size < 1024)
    {
        return {
                .size = new_size,
                .prefix = ""
        };
    }
    else if (size >= 1024 && size < 1024 * 1024)
    {
        return {
                .size = new_size / 1024,
                .prefix = "K"
        };
    }
    else if (size >= 1024 * 1024 && size < 1024 * 1024 * 1024)
    {
        return {
                .size = new_size / (1024 * 1024),
                .prefix = "M"
        };
    }
    else if (size >= 1024LL * 1024LL * 1024LL && size < 1024LL * 1024LL * 1024LL * 1024LL)
    {
        return {
                .size = new_size / (1024LL * 1024LL * 1024LL),
                .prefix = "G"
        };
    }
    else if (size >= 1024LL * 1024LL * 1024LL * 1024LL)
    {
        return {
                .size = new_size / (1024LL * 1024LL * 1024LL * 1024LL),
                .prefix = "T"
        };
    }

    return {
            .size = new_size,
            .prefix = "OOB"
    };
}

void pmm_print_unaligned_memory_map_entries()
{
    kstd::printf("[PMM] Unaligned memory entries  (to the page): \n");
    for (size_t i = 0; i < pmm_limine_memmap_entry_count; i++)
    {
        const limine_memmap_entry* entry = pmm_limine_memmap_entries[i];

        if (entry->base % PAGE_SIZE != 0)
        {
            kstd::printf("    "); // Add padding.
            pmm_print_memmap_entry(i, entry);
        }
    }
    kstd::printf("[[ END ]]\n");
}

void pmm_print_memory_usage()
{
    mem_size overall_size = pmm_calculate_effective_size(pmm_overall_memory);
    kstd::printf("[PMM] Overall memory: %f (%sB)\n", overall_size.size, overall_size.prefix);

    mem_size free_memory = pmm_calculate_effective_size(pmm_usable_memory);
    kstd::printf("[PMM] Free memory: %f (%sB)\n", free_memory.size, free_memory.prefix);

    mem_size rsvd_memory = pmm_calculate_effective_size(pmm_reserved_memory);
    kstd::printf("[PMM] Reserved memory: %f (%sB)\n", rsvd_memory.size, rsvd_memory.prefix);

    mem_size bad_memory = pmm_calculate_effective_size(pmm_bad_memory);
    kstd::printf("[PMM] Bad memory: %f (%sB)\n", bad_memory.size, bad_memory.prefix);

    mem_size unusable_memory = pmm_calculate_effective_size(pmm_unusable_memory);
    kstd::printf("[PMM] Unusable memory: %f (%sB)\n", unusable_memory.size, unusable_memory.prefix);

    mem_size free_pages = pmm_calculate_effective_size(pmm_usable_memory / PAGE_SIZE);
    kstd::printf("[PMM] Free pages: %f [%sP]\n", free_pages.size, free_pages.prefix);
}

static uint64_t pmm_get_maximum_address()
{
    static uint64_t largest_address = 0;

    if (largest_address) return largest_address; // cache it. it's genius isn't it?

    for (size_t i = 0; i < pmm_limine_memmap_entry_count; i++)
    {
        const limine_memmap_entry* entry = pmm_limine_memmap_entries[i];

        if ((entry->base + entry->length) > largest_address)
        {
            largest_address = entry->base + entry->length;
        }
    }

    return largest_address;
}

uint64_t pmm_round_to_next_page(uint64_t address)
{
    return (address + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
}

void pmm_init()
{
    static bool run_once = []() {
        if (pmm_limine_memmap_request.response == nullptr)
        {
            if constexpr (pmm_verbose)
                kstd::printf("[PMM] Failed to initialize Physical Memory Manager. Reason: Limine's memory map request response is null.\n");

            return false;
        }

        pmm_limine_memmap_response = pmm_limine_memmap_request.response;
        pmm_limine_memmap_entries = pmm_limine_memmap_response->entries;
        pmm_limine_memmap_entry_count = pmm_limine_memmap_response->entry_count;

        pmm_calculate_memory(pmm_limine_memmap_entries, pmm_limine_memmap_entry_count);

        // Find the max address.
        uint64_t max_address = pmm_get_maximum_address();

        if constexpr (pmm_verbose)
        {
            kstd::printf("[PMM] The largest address: %lx.\n", max_address);

            mem_size _size = pmm_calculate_effective_size(max_address);
            kstd::printf("[PMM] Boundaries of memory: 0 - %f (%sB).\n", _size.size, _size.prefix);
        }

        // Since we got the max address we can calculate the pages that the address range holds. Round up.
        uint64_t rounded_max_address = pmm_round_to_next_page(max_address);

        if constexpr (pmm_verbose)
        {
            kstd::printf("[PMM] The rounded address is: %lx.\n", rounded_max_address);

            mem_size _size = pmm_calculate_effective_size(max_address);
            kstd::printf("[PMM] New rounded boundaries of memory: 0 - %f (%sB).\n", _size.size, _size.prefix);
        }

        size_t bitmap_size = rounded_max_address / (PAGE_SIZE * 8); // Bytes

        if constexpr (pmm_verbose)
        {
            mem_size _size = pmm_calculate_effective_size(bitmap_size);
            kstd::printf("[PMM] Space required for the bitmap: %f (%sB)\n", _size.size, _size.prefix);
        }

        if (bitmap_size % PAGE_SIZE != 0)
        {
            if constexpr (pmm_verbose)
            {
                kstd::printf("[PMM] The size of bitmap isn't aligned to the page.\n[PMM] Realigning.\n");
            }

            bitmap_size = pmm_round_to_next_page(bitmap_size);
        }

        // Bitmap size is aligned to the page. We can start looking for place for the bitmap to store.
        for (size_t i = 0; i < pmm_limine_memmap_entry_count; i++)
        {
            limine_memmap_entry* entry = pmm_limine_memmap_entries[i];

            // We're checking if entry is:
            // - Usable
            // - Size is greater or equal than the size of our bitmap.
            // - If the entry is aligned.
            if (entry->type == LIMINE_MEMMAP_USABLE && bitmap_size <= entry->length && (entry->base % PAGE_SIZE == 0))
            {
                if constexpr(pmm_verbose)
                {
                    kstd::printf("[PMM] Found perfect entry for the bitmap.\n");
                    pmm_print_memmap_entry(i, entry);

                    pmm_memory_bitmap_size = bitmap_size;
                    pmm_memory_bitmap_raw = vmm_make_virtual<decltype(pmm_memory_bitmap_raw)>(entry->base);

                    entry->length -= bitmap_size;
                    entry->base += bitmap_size;

                    break;
                }
            }
        }

        // recalculate.
        pmm_calculate_memory(pmm_limine_memmap_entries, pmm_limine_memmap_entry_count);
        pmm_lock_memory_recalculation();

        if constexpr (pmm_verbose)
        {
            kstd::printf("[PMM] The bitmap address is: %lx.\n", pmm_memory_bitmap_raw);
        }

        if (pmm_memory_bitmap_raw == nullptr)
        {
            kstd::printf("[PMM] Bitmap address is null. Killing system.\n");

            bochs_breakpoint();
            unreachable();
        }

        kstd::printf("BITMAP ADDRESS: %llx\n", pmm_memory_bitmap_raw);

        pmm_bitmap_controller.Initialize(pmm_memory_bitmap_raw, pmm_memory_bitmap_size);

        kstd::memset(pmm_memory_bitmap_raw, 0xff, pmm_memory_bitmap_size);

        // fill the bitmap with goodies.
        for (size_t i = 0; i < pmm_limine_memmap_entry_count; i++)
        {
            const limine_memmap_entry* entry = pmm_limine_memmap_entries[i];

            switch (entry->type)
            {
                case LIMINE_MEMMAP_USABLE:
                    pmm_bitmap_controller.unmark_addrs_in_range(entry->base, entry->length);
                    break;
                default:
                    pmm_bitmap_controller.mark_addrs_used_in_range(entry->base, entry->length);
                    break;
            }
        }

        if constexpr (pmm_verbose)
        {
            kstd::printf("[PMM] Finished!\n");
        }

        return true;
    }();
}


void PMMBitmap::mark_page(uint64_t page_index)
{
    this->Set(page_index);
}

void PMMBitmap::unmark_page(uint64_t page_index)
{
    this->Clear(page_index);
}

void PMMBitmap::mark_addr(uint64_t address)
{
    uint64_t page_idx = calculate_page_index(address);
    this->Set(page_idx);
}

void PMMBitmap::unmark_addr(uint64_t address)
{
    uint64_t page_idx = calculate_page_index(address);
    this->Clear(page_idx);
}

void PMMBitmap::mark_pages_used_in_range(uint64_t page_index, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        this->Set(page_index + i);
    }
}

void PMMBitmap::mark_addrs_used_in_range(uint64_t address, size_t len)
{
    uint64_t end_address = address + len;
    while (address < end_address)
    {
        this->mark_addr(address);
        address += PAGE_SIZE;
    }
}

void PMMBitmap::unmark_pages_in_range(uint64_t page_index, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        this->Clear(page_index + i);
    }
}

void PMMBitmap::unmark_addrs_in_range(uint64_t address, size_t len)
{
    uint64_t end_address = address + len;
    while (address < end_address)
    {
        this->unmark_addr(address);
        address += PAGE_SIZE;
    }
}

uint64_t pmm_alloc_page()
{
    uint64_t addr = pmm_bitmap_controller.FindFirstCleared() * 4096;
    if (addr == 0)
    {
        kstd::printf("pmm_alloc_page(): OOM\n");
        unreachable();
    }
    pmm_bitmap_controller.Set(addr / 4096);
    pmm_usable_memory -= 4096;
    // kstd::printf("Usable memory: %f [%%]\n", ((double)pmm_usable_memory / (double)pmm_overall_memory) * 100);
    return addr;
}

void pmm_free_page(uint64_t addr)
{
    uint64_t idx = addr / 4096;
    pmm_bitmap_controller.Clear(idx);
    pmm_usable_memory += 4096;
}