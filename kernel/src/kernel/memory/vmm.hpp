//
// Created by Piotr on 18.05.2024.
//

#ifndef KITTY_OS_CPP_VMM_HPP
#define KITTY_OS_CPP_VMM_HPP

#include <kernel/memory/pmm.hpp>
#include <type_traits>

/*
 * structures
 */
struct vmm_address
{
    uint64_t padding : 16;
    uint64_t pml4e : 9;
    uint64_t pdpe : 9;
    uint64_t pde : 9;
    uint64_t pte : 9;
    uint64_t offset : 12;
} __attribute__((packed));

struct pml4e
{
    uint64_t present : 1;
    uint64_t read_write : 1;
    uint64_t user_supervisor : 1;
    uint64_t page_write_through : 1;
    uint64_t page_cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t ignored : 1;
    uint64_t must_be_zero : 2;
    uint64_t available_to_software1 : 3;
    uint64_t pdpe_ptr : 40;
    uint64_t available_to_software2 : 11;
    uint64_t no_execute : 1;
} __attribute__((packed));

struct pdpe
{
    uint64_t present : 1;
    uint64_t read_write : 1;
    uint64_t user_supervisor : 1;
    uint64_t page_write_through : 1;
    uint64_t page_cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t ignored : 1;
    uint64_t zero : 1;
    uint64_t ignored1 : 1;
    uint64_t available_to_software1 : 3;
    uint64_t pde_ptr : 40;
    uint64_t available_to_software2 : 11;
    uint64_t no_execute : 1;
} __attribute__((packed));

struct pde
{
    uint64_t present : 1;
    uint64_t read_write : 1;
    uint64_t user_supervisor : 1;
    uint64_t page_write_through : 1;
    uint64_t page_cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t ignored : 1;
    uint64_t zero : 1;
    uint64_t ignored1 : 1;
    uint64_t available_to_software1 : 3;
    uint64_t pte_ptr : 40;
    uint64_t available_to_software2 : 11;
    uint64_t no_execute : 1;
} __attribute__((packed));

struct pte
{
    uint64_t present : 1;
    uint64_t read_write : 1;
    uint64_t user_supervisor : 1;
    uint64_t page_write_through : 1;
    uint64_t page_cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t page_attribute_table : 1;
    uint64_t global : 1;
    uint64_t available_to_software1 : 3;
    uint64_t phys_ptr : 40;
    uint64_t available_to_software2 : 7;
    uint64_t protection_keys : 4;
    uint64_t no_execute : 1;
} __attribute__((packed));

/*
 * enums
 */
typedef enum : int {
    PROT_NONE = 0,
    PROT_RW = 1 << 0,
    PROT_NOEXEC = 1 << 1,
    PROT_SUPERVISOR = 1 << 2, // If SU = 0. Only supervisor has access, if SU = 1, user and supervisor has access.
} PROTECTION_FLAGS;

typedef enum : int {
    MAP_NONE = 0,
    MAP_PRESENT = 1 << 0,
    MAP_GLOBAL = 1 << 1
} MAP_FLAGS;

typedef enum : int {
    MISC_NONE = 0,
    MISC_INVLPG = 1 << 0,
    MISC_FORCE = 1 << 1
} MISC_FLAGS;

/*
 * constexpr things
 */

static constexpr bool vmm_verbose = true;

/*
 * inline functions
 */
inline uint64_t vmm_read_cr3()
{
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

inline uint64_t vmm_get_pml4()
{
    return vmm_read_cr3() & (0xfffffffffffff000);
}

template <typename T>
constexpr vmm_address vmm_split_va(T vaddr) {
    // Assuming vaddr is a virtual memory address represented as an integer type
    vmm_address result;

    // Extract different components of the virtual address
    result.padding = (vaddr >> 48) & 0xFFFF;
    result.pml4e = (vaddr >> 39) & 0x1FF;
    result.pdpe = (vaddr >> 30) & 0x1FF;
    result.pde = (vaddr >> 21) & 0x1FF;
    result.pte = (vaddr >> 12) & 0x1FF;
    result.offset = vaddr & 0xFFF;

    // Return the result
    return result;
}


/*
 * externs.
 */
extern limine_hhdm_response* vmm_hhdm;

/*
 * Global function definitions
 */
void vmm_init();


/*
 * templates
 */

template <typename T1, typename T2>
T1 vmm_make_virtual(T2 pma)
{
    uint64_t pa = static_cast<uint64_t>(pma);
    uint64_t va = pa + vmm_hhdm->offset;
    return (T1)(va);
}

// paging types: pml5e and pml4e. (pml5e unsupported for now)
bool vmm_map(pml4e* pml4e, uint64_t virt_address, uint64_t phys_address, int prot_flags, int map_flags, int misc_flags);

static inline void flush_tlb(unsigned long addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

#endif //KITTY_OS_CPP_VMM_HPP
