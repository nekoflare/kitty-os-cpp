//
// Created by Piotr on 18.05.2024.
//

#ifndef KITTY_OS_CPP_VMM_HPP
#define KITTY_OS_CPP_VMM_HPP

#include <kernel/memory/pmm.hpp>

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
typedef enum {
    PROT_RW = 1 << 0,
    PROT_NOEXEC = 1 << 1,
    PROT_SUPERVISOR = 1 << 2, // If SU = 0. Only supervisor has access, if SU = 1, user and supervisor has access.
} PROTECTION_FLAGS;

typedef enum {
    MAP_PRESENT = 1 << 0,
    MAP_GLOBAL = 1 << 1
} MAP_FLAGS;

typedef enum {
    MISC_INVLPG = 1 << 0,
    MISC_FORCE = 1 << 1
} MISC_FLAGS;

/*
 * templates
 */

// paging types: pml5e and pml4e. (pml5e unsupported for now)
template <typename PagingType, typename VirtualType, typename PhysicalType>
bool vmm_map(PagingType* pml4e, VirtualType virt_address, PhysicalType phys_address, PROTECTION_FLAGS prot_flags, MAP_FLAGS map_flags, MISC_FLAGS misc_flags)
{
    
}
/*
 * inline functions
 */
inline uint64_t vmm_read_cr3()
{
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

inline pml4e* vmm_get_pml4()
{
    return reinterpret_cast<pml4e*>(vmm_read_cr3() & (0xfffffffffffff000));
}

/*
 * externs.
 */
extern limine_hhdm_response* vmm_hhdm;

/*
 * Templates
 */
template <typename T1, typename T2>
T1 vmm_make_virtual(T2 pma)
{
    return reinterpret_cast<T1>(reinterpret_cast<uint64_t>(pma) + vmm_hhdm->offset);
}

/*
 * Global function definitions
 */
void vmm_init();

#endif //KITTY_OS_CPP_VMM_HPP
