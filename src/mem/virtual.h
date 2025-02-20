//
// Created by Neko on 20.01.2025.
//

#ifndef VIRTUAL_H
#define VIRTUAL_H

#include <cstdint>
#include <utility>

#include "dbg/log.h"
#include "physical.h"

#define ALIGN_DOWN(addr, align) ((addr) & ~((align)-1))
#define ALIGN_UP(addr, align) (((addr) + (align)-1) & ~((align)-1))

uintptr_t get_higher_half_memory_offset();

void initialize_virtual_memory();

struct virtual_address
{
    uint64_t canonical : 16;
    uint64_t pml4 : 9;
    uint64_t pdp : 9;
    uint64_t pd : 9;
    uint64_t pt : 9;
    uint64_t offset : 12;
} __attribute__((packed));

constexpr uint64_t structure_to_virtual_address(const virtual_address address)
{
    return ((address.canonical & 0xFFFFULL) << 48ULL) | ((address.pml4 & 0x1FFULL) << 39ULL) |
           ((address.pdp & 0x1FFULL) << 30ULL) | ((address.pd & 0x1FFULL) << 21ULL) |
           ((address.pt & 0x1FFULL) << 12ULL) | (address.offset & 0xFFFULL);
}

inline void *get_current_page_map_address()
{
    uint64_t cr3 = 0;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    cr3 &= 0xfffffffffffff000;
    cr3 += get_higher_half_memory_offset();
    return reinterpret_cast<void *>(cr3);
}

inline bool invalidate_pages_in_range(uint64_t range_start, uint64_t range_end)
{
    range_start &= ~(PAGE_SIZE - 1);
    range_end = (range_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    if (range_start > range_end)
    {
        debug_print("Range start is greater than range end.");
        return false;
    }

    for (uint64_t addr = range_start; addr < range_end; addr += PAGE_SIZE)
    {
        asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
    }

    return true;
}

struct pml4
{
    uint64_t p : 1;        // present
    uint64_t rw : 1;       // read write
    uint64_t us : 1;       // user / supervisor
    uint64_t pwt : 1;      // page write through
    uint64_t pcd : 1;      // page cache disable
    uint64_t a : 1;        // accessed
    uint64_t ign0 : 1;     // ignored
    uint64_t mbz0 : 2;     // must be zero
    uint64_t ats0 : 3;     // available to software
    uint64_t pdp_ppn : 40; // pdp page index
    uint64_t ats1 : 11;    // available to software
    uint64_t nx : 1;       // no execute
} __attribute__((packed));

struct pdp
{
    uint64_t p : 1;       // present
    uint64_t rw : 1;      // read write
    uint64_t us : 1;      // user / supervisor
    uint64_t pwt : 1;     // page write through
    uint64_t pcd : 1;     // page cache disable
    uint64_t a : 1;       // accessed
    uint64_t ign0 : 1;    // ignored
    uint64_t z : 1;       // zero
    uint64_t ign1 : 1;    // ignored
    uint64_t ats0 : 3;    // available to software
    uint64_t pd_ppn : 40; // pd page index
    uint64_t ats1 : 11;   // available to software
    uint64_t nx : 1;      // no execute
} __attribute__((packed));

struct pd
{
    uint64_t p : 1;       // present
    uint64_t rw : 1;      // read write
    uint64_t us : 1;      // user / supervisor
    uint64_t pwt : 1;     // page write through
    uint64_t pcd : 1;     // page cache disable
    uint64_t a : 1;       // accessed
    uint64_t ign0 : 1;    // ignored
    uint64_t z : 1;       // zero
    uint64_t ign1 : 1;    // ignored
    uint64_t ats0 : 3;    // available to software
    uint64_t pt_ppn : 40; // pt page index
    uint64_t ats1 : 11;   // available to software
    uint64_t nx : 1;      // no execute
} __attribute__((packed));

struct pt
{
    uint64_t p : 1;         // present
    uint64_t rw : 1;        // read write
    uint64_t us : 1;        // user / supervisor
    uint64_t pwt : 1;       // page write through
    uint64_t pcd : 1;       // page cache disable
    uint64_t a : 1;         // accessed
    uint64_t d : 1;         // dirty
    uint64_t pat : 1;       // page attribute table
    uint64_t g : 1;         // global
    uint64_t ats0 : 3;      // available to software
    uint64_t phys_ppn : 40; // physical address page index
    uint64_t ats1 : 7;      // available to software
    uint64_t pk : 4;        // protection keys
    uint64_t nx : 1;        // no execute
} __attribute__((packed));

constexpr virtual_address split_virtual_address_to_structure(const uint64_t address)
{
    virtual_address result{};

    result.canonical = (address >> 48) & 0xFFFF;
    result.pml4 = (address >> 39) & 0x1FF;
    result.pdp = (address >> 30) & 0x1FF;
    result.pd = (address >> 21) & 0x1FF;
    result.pt = (address >> 12) & 0x1FF;
    result.offset = address & 0xFFF;

    return result;
}

inline void invalidate_page(uint64_t address)
{
    asm volatile("invlpg (%0)" ::"r"(address) : "memory");
}

template <typename FilterFunc>
bool ensure_virtual_memory_flags(pml4 *page_map, uint64_t virtual_address, FilterFunc &&filter)
{
    auto indexes = split_virtual_address_to_structure(virtual_address);
    uintptr_t hhdm = get_higher_half_memory_offset();

    // Check PML4 entry
    auto &pml4e = page_map[indexes.pml4];
    if (!pml4e.p)
        return false;

    // Access PDP table
    pdp *pdp_table = reinterpret_cast<pdp *>((pml4e.pdp_ppn << 12) + hhdm);
    auto &pdpe = pdp_table[indexes.pdp];
    if (!pdpe.p)
        return false;

    // Access PD table
    pd *pd_table = reinterpret_cast<pd *>((pdpe.pd_ppn << 12) + hhdm);
    auto &pde = pd_table[indexes.pd];
    if (!pde.p)
        return false;

    // Access PT table
    pt *pt_table = reinterpret_cast<pt *>((pde.pt_ppn << 12) + hhdm);
    auto &pte = pt_table[indexes.pt];
    if (!pte.p)
        return false;

    // Apply filter function to validate the entry
    return filter(pte);
}

template <typename FilterFunc>
bool ensure_virtual_memory_flags_in_range(pml4 *page_map, uint64_t range_start, uint64_t range_end, FilterFunc &&filter)
{
    range_start = ALIGN_DOWN(range_start, PAGE_SIZE);
    range_end = ALIGN_UP(range_end, PAGE_SIZE);

    if (range_start > range_end)
    {
        return false;
    }

    for (uint64_t addr = range_start; addr < range_end; addr += 0x1000)
    {
        if (!ensure_virtual_memory_flags(page_map, addr, filter))
        {
            return false;
        }
    }

    return true;
}

bool virtual_map(pml4 *page_map, uint64_t physical_address, uint64_t virtual_address, bool is_read_write,
                 bool disable_execution, bool is_supervisor, bool page_write_through, bool page_cache_disable,
                 bool page_attribute_table, uint8_t protection_keys, bool is_present);

void print_page_info(uint64_t virtual_address);
uint64_t allocate_virtual_memory_kernel(uint64_t size);

pml4 *get_kernel_page_map();
bool set_page_map(pml4 *new_page_map);

std::pair<bool, uint64_t> virtual_to_physical(const pml4 *page_map, uint64_t virtual_address);

#endif // VIRTUAL_H
