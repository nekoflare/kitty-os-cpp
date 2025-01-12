#pragma once

#include <stdint.h>

struct virtual_address
{
    uint64_t canonical : 16;
    uint64_t pml4 : 9;
    uint64_t pdp : 9;
    uint64_t pd : 9;
    uint64_t pt : 9;
    uint64_t offset : 12;
} __attribute__((packed));

constexpr uint64_t structure_to_virtual_address(virtual_address va)
{
    return ((va.canonical & 0xFFFFULL) << 48ULL) | ((va.pml4 & 0x1FFULL) << 39ULL) | ((va.pdp & 0x1FFULL) << 30ULL) |
           ((va.pd & 0x1FFULL) << 21ULL) | ((va.pt & 0x1FFULL) << 12ULL) | (va.offset & 0xFFFULL);
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

constexpr virtual_address split_virtual_address_to_structure(uint64_t address)
{
    virtual_address result;

    result.canonical = (address >> 48) & 0xFFFF;
    result.pml4 = (address >> 39) & 0x1FF;
    result.pdp = (address >> 30) & 0x1FF;
    result.pd = (address >> 21) & 0x1FF;
    result.pt = (address >> 12) & 0x1FF;
    result.offset = address & 0xFFF;

    return result;
}

inline void invlpg(uint64_t address)
{
    asm volatile("invlpg (%0)" ::"r"(address) : "memory");
}

void initialize_virtual_memory(void);
bool virtual_map(uint64_t physical_address, uint64_t virtual_address, bool is_read_write, bool disable_execution,
                 bool is_supervisor, bool pwt, bool pcd, bool pat, uint8_t pk, bool is_present);

void print_page_info(uint64_t virtual_address);
uint64_t allocate_virtual_memory_kernel(uint64_t size);
void allocate_to(uint64_t virtual_base, uint64_t bytes);