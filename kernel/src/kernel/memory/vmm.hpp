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
 * const expr things
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
inline vmm_address vmm_split_va(T vaddr)
{
    uint64_t va = reinterpret_cast<uint64_t>(vaddr);
    return {
            .padding = (va >> 48) & 0xffff,
            .pml4e = (va >> 38) & 0x1ff,
            .pdpe = (va >> 29) & 0x1ff,
            .pde = (va >> 21) & 0x1ff,
            .pte = (va >> 12) & 0x1ff,
            .offset = va & 0xfff,
    };
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


/*
 * templates
 */

// paging types: pml5e and pml4e. (pml5e unsupported for now)
template <typename PagingType, typename VirtualType, typename PhysicalType>
bool vmm_map(PagingType pmle, VirtualType virt_address, PhysicalType phys_address, PROTECTION_FLAGS prot_flags, MAP_FLAGS map_flags, MISC_FLAGS misc_flags)
{
    if constexpr (std::is_same_v<decltype(pmle), pml4e*>)
    {
        pml4e* pml4e_ptr = pmle;

        // Convert virt_address to uint64_t if it's not already
        uint64_t vaddr;
        if constexpr (std::is_convertible_v<VirtualType, uint64_t>)
        {
            vaddr = static_cast<uint64_t>(virt_address);
        }
        else
        {
            vaddr = reinterpret_cast<uint64_t>(virt_address);
        }

        // Convert phys_address to uint64_t if it's not already
        uint64_t paddr;
        if constexpr (std::is_convertible_v<PhysicalType, uint64_t>)
        {
            paddr = static_cast<uint64_t>(phys_address);
        }
        else
        {
            paddr = reinterpret_cast<uint64_t>(phys_address);
        }

        if ((vaddr & 0xffff000000000000) >> 48 != 0x0 && (vaddr & 0xffff000000000000) >> 48 != 0xffff)
        {
            if constexpr (vmm_verbose)
            {
                kstd::printf("[VMM] Couldn't map it. It's not in kernel or in user mode. Bits aren't matching.\n");
            }
            return false;
        }

        // Split vaddr in to pieces
        vmm_address addr = vmm_split_va(vaddr);

        uint64_t pml4e_idx = addr.pml4e;
        uint64_t pdpe_idx = addr.pdpe;
        uint64_t pde_idx = addr.pde;
        uint64_t pte_idx = addr.pte;

        // Check if pml4e entry is present.
        if (pml4e_ptr[pml4e_idx].present == 0)
        {
            if constexpr (vmm_verbose)
                kstd::printf("[VMM] PML4e entry isn't present. %lx\n", pml4e_ptr[pml4e_idx].pdpe_ptr << 12);

            return false;
        }

        pdpe* pdpe_ptr = vmm_make_virtual<pdpe*>(pml4e_ptr[pml4e_idx].pdpe_ptr << 12);

        // check if pdpe entry is present.
        if (pdpe_ptr[pdpe_idx].present == 0)
        {
            if constexpr (vmm_verbose)
                kstd::printf("[VMM] PDPe entry isn't present. %lx\n", pdpe_ptr[pdpe_idx].pde_ptr);

            return false;
        }

        return true; // Assuming the mapping is successful

    }
    else
    {
        if constexpr (vmm_verbose)
            kstd::printf("Unsupported.\n");
        return false;
    }
}

#endif //KITTY_OS_CPP_VMM_HPP
