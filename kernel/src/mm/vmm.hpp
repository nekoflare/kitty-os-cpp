//
// Created by Piotr on 18.05.2024.
//

#ifndef KITTY_OS_CPP_VMM_HPP
#define KITTY_OS_CPP_VMM_HPP

#include <mm/pmm.hpp>
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

inline uint64_t vmm_sva_to_va(vmm_address va)
{
    return ((va.padding & 0xFFFFULL) << 48ULL) |
           ((va.pml4e & 0x1FFULL) << 39ULL) |
           ((va.pdpe & 0x1FFULL) << 30ULL) |
           ((va.pde & 0x1FFULL) << 21ULL) |
           ((va.pte & 0x1FFULL) << 12ULL) |
           (va.offset & 0xFFFULL);
}

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
constexpr vmm_address vmm_split_va(T va) {
    // Assuming vaddr is a virtual memory address represented as an integer type
    vmm_address result;

    uint64_t vaddr = reinterpret_cast<uint64_t>(va);

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
    return reinterpret_cast<T1>(va);
}

template <typename T>
T vmm_make_virtual_singular(T v)
{
    uint64_t pv = reinterpret_cast<uint64_t>(v);
    uint64_t va = pv + vmm_hhdm->offset;
    return reinterpret_cast<T>(va);
}

// paging types: pml5e and pml4e. (pml5e unsupported for now)
bool vmm_map(pml4e* pml4e, uint64_t virt_address, uint64_t phys_address, int prot_flags, int map_flags, int misc_flags);

static inline  void flush_tlb(unsigned long addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

void vmm_test(void* addr);

constexpr uint64_t vmm_create_virtual_address(bool is_user, uint64_t pml4e_index, uint64_t pdpe_index, uint64_t pde_index, uint64_t pte_index, uint64_t offset)
{
    uint64_t virtual_address = 0;

    if (!is_user) {
        virtual_address |= 0xFFFFULL << 48;
    }

    virtual_address |= (pml4e_index & 0x1FFULL) << 39;
    virtual_address |= (pdpe_index & 0x1FFULL) << 30;
    virtual_address |= (pde_index & 0x1FFULL) << 21;
    virtual_address |= (pte_index & 0x1FFULL) << 12;
    virtual_address |= (offset & 0xFFFULL);

    return virtual_address;
}


#endif //KITTY_OS_CPP_VMM_HPP
