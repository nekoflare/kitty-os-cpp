//
// Created by Neko on 20.01.2025.
//

#include "virtual.h"

#include <cstring>
#include <dbg/log.h>
#include <limine.h>

#include "physical.h"

limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = nullptr,
};

limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
    .response = nullptr,
};

uint64_t get_kernel_base_address()
{
    if (kernel_address_request.response == nullptr)
    {
        debug_print("Kernel address request is null\n");
        return 0;
    }

    return kernel_address_request.response->virtual_base;
}

uintptr_t get_higher_half_memory_offset()
{
    if (hhdm_request.response == nullptr)
    {
        debug_print("Higher half memory offset is null\n");
        return 0;
    }

    return hhdm_request.response->offset;
}

uint64_t base_address = 0;
uint64_t remaining_bytes = 0;

pml4 *kernel_pml4;

void initialize_virtual_memory()
{
    base_address = get_higher_half_memory_offset() + get_highest_address();
    remaining_bytes = get_kernel_base_address() - base_address;
    debug_print("Bytes in the hole: %016lX (B)\n", remaining_bytes);
    kernel_pml4 = static_cast<pml4 *>(get_current_page_map_address());
}

pml4 *get_kernel_page_map()
{
    return kernel_pml4;
}

bool set_page_map(pml4 *new_page_map)
{
    uint64_t page_map_register = 0;
    asm volatile("mov %%cr3, %0" : "=r"(page_map_register));
    page_map_register &= 0xfff; // Get only flags

    auto [success, physical_address] = virtual_to_physical(get_kernel_page_map(),
                                                           reinterpret_cast<uint64_t>(new_page_map));

    if (!success)
    {
        debug_print("==[ERROR]== Failed to set the page map. Couldn't get the "
                    "physical address of the current page map.\n");
        return false;
    }
    page_map_register |= physical_address; // keep it as is. or we overwrite the flags haha

    asm volatile("mov %0, %%cr3" :: "r"(page_map_register) : "memory");
    return true;
}

std::pair<bool, uint64_t> virtual_to_physical(const pml4 *page_map, const uint64_t virtual_address)
{
    debug_print("========== Starting virtual_to_physical(page_map=%p, virtual_address=0x%lx) ==========\n",
                page_map, virtual_address);

    // Check if the address is canonical
    debug_print("========== Checking Canonical Form ==========\n");
    auto is_canonical = [](const uint64_t addr) -> bool {
        const uint64_t high_bits = addr >> 48;
        return high_bits == 0 || high_bits == 0xFFFF;
    };
    debug_print("Virtual address high bits (>> 48): 0x%lx\n", virtual_address >> 48);
    if (!is_canonical(virtual_address))
    {
        debug_print("Failed: Virtual address is not canonical. Address: 0x%llx\n", virtual_address);
        debug_print("========== End of virtual_to_physical (Failed: Non-canonical) ==========\n");
        return {false, 0};
    }
    debug_print("Address is canonical.\n");
    debug_print("========== End of Canonical Check ==========\n");

    // Split the virtual address into its components
    debug_print("========== Splitting Virtual Address ==========\n");
    const auto indexes = split_virtual_address_to_structure(virtual_address);
    debug_print("Virtual address breakdown:\n");
    debug_print("  Canonical: 0x%lx\n", indexes.canonical);
    debug_print("  PML4 Index: %lu (0x%lx)\n", indexes.pml4, indexes.pml4);
    debug_print("  PDP Index: %lu (0x%lx)\n", indexes.pdp, indexes.pdp);
    debug_print("  PD Index: %lu (0x%lx)\n", indexes.pd, indexes.pd);
    debug_print("  PT Index: %lu (0x%lx)\n", indexes.pt, indexes.pt);
    debug_print("  Offset: 0x%lx\n", indexes.offset);
    debug_print("========== End of Address Split ==========\n");

    const uintptr_t hhdm = get_higher_half_memory_offset();
    debug_print("Higher Half Direct Map offset (HHDM): 0x%lx\n", hhdm);

    // PML4 Level
    debug_print("========== PML4 Level ==========\n");
    debug_print("Accessing PML4 at %p, index %lu\n", page_map, indexes.pml4);
    auto &pml4e = page_map[indexes.pml4];
    debug_print("PML4 Entry: P=%lu, RW=%lu, US=%lu, PWT=%lu, PCD=%lu, A=%lu, PDP_PPN=0x%lx, NX=%lu\n",
                pml4e.p, pml4e.rw, pml4e.us, pml4e.pwt, pml4e.pcd, pml4e.a, pml4e.pdp_ppn, pml4e.nx);
    if (!pml4e.p)
    {
        debug_print("Failed: PML4 entry not present. Index: %lu\n", indexes.pml4);
        debug_print("========== End of virtual_to_physical (Failed: PML4) ==========\n");
        return {false, 0};
    }
    const auto pdp_table = reinterpret_cast<volatile pdp *>((pml4e.pdp_ppn << 12) + hhdm);
    debug_print("PML4 PDP_PPN shifted: 0x%lx, PDP table address: %p\n", (pml4e.pdp_ppn << 12), pdp_table);
    debug_print("========== End of PML4 Level ==========\n");

    // PDP Level
    debug_print("========== PDP Level ==========\n");
    debug_print("Accessing PDP at %p, index %lu\n", pdp_table, indexes.pdp);
    const auto &pdpe = pdp_table[indexes.pdp];
    debug_print("PDP Entry: P=%lu, RW=%lu, US=%lu, PWT=%lu, PCD=%lu, A=%lu, PD_PPN=0x%lx, NX=%lu\n",
                pdpe.p, pdpe.rw, pdpe.us, pdpe.pwt, pdpe.pcd, pdpe.a, pdpe.pd_ppn, pdpe.nx);
    if (!pdpe.p)
    {
        debug_print("Failed: PDP entry not present. Index: %lu\n", indexes.pdp);
        debug_print("========== End of virtual_to_physical (Failed: PDP) ==========\n");
        return {false, 0};
    }
    const auto pd_table = reinterpret_cast<volatile pd *>((pdpe.pd_ppn << 12) + hhdm);
    debug_print("PDP PD_PPN shifted: 0x%lx, PD table address: %p\n", (pdpe.pd_ppn << 12), pd_table);
    debug_print("========== End of PDP Level ==========\n");

    // PD Level
    debug_print("========== PD Level ==========\n");
    debug_print("Accessing PD at %p, index %lu\n", pd_table, indexes.pd);
    const auto &pde = pd_table[indexes.pd];
    debug_print("PD Entry: P=%lu, RW=%lu, US=%lu, PWT=%lu, PCD=%lu, A=%lu, PT_PPN=0x%lx, NX=%lu\n",
                pde.p, pde.rw, pde.us, pde.pwt, pde.pcd, pde.a, pde.pt_ppn, pde.nx);
    if (!pde.p)
    {
        debug_print("Failed: PD entry not present. Index: %lu\n", indexes.pd);
        debug_print("========== End of virtual_to_physical (Failed: PD) ==========\n");
        return {false, 0};
    }
    const auto pt_table = reinterpret_cast<volatile pt *>((pde.pt_ppn << 12) + hhdm);
    debug_print("PD PT_PPN shifted: 0x%lx, PT table address: %p\n", (pde.pt_ppn << 12), pt_table);
    debug_print("========== End of PD Level ==========\n");

    // PT Level
    debug_print("========== PT Level ==========\n");
    debug_print("Accessing PT at %p, index %lu\n", pt_table, indexes.pt);
    const auto &pte = pt_table[indexes.pt];
    debug_print("PT Entry: P=%lu, RW=%lu, US=%lu, PWT=%lu, PCD=%lu, PAT=%lu, PK=%lu, NX=%lu, PHYS_PPN=0x%lx\n",
                pte.p, pte.rw, pte.us, pte.pwt, pte.pcd, pte.pat, pte.pk, pte.nx, pte.phys_ppn);
    if (pte.p <= 0)
    {
        debug_print("Failed: PT entry not present. Index: %lu\n", indexes.pt);
        debug_print("========== End of virtual_to_physical (Failed: PT) ==========\n");
        return {false, 0};
    }

    // Calculate physical address
    debug_print("========== Calculating Physical Address ==========\n");
    uint64_t phys = (pte.phys_ppn << 12) | indexes.offset;
    debug_print("Physical address calculation: (PHYS_PPN << 12) = 0x%lx | Offset = 0x%lx = 0x%lx\n",
                (pte.phys_ppn << 12), indexes.offset, phys);
    debug_print("Success: Virtual address 0x%lx maps to physical address 0x%lx\n", virtual_address, phys);
    debug_print("========== End of virtual_to_physical (Success) ==========\n");

    return {true, phys};
}

bool virtual_map(pml4 *page_map, uint64_t physical_address, uint64_t virtual_address, bool is_read_write,
                 bool disable_execution, bool is_supervisor, bool page_write_through, bool page_cache_disable,
                 bool page_attribute_table, uint8_t protection_keys, bool is_present)
{
    // Helper function: Check if an address is canonical
    auto is_canonical = [](uint64_t addr) -> bool {
        uint64_t high_bits = addr >> 48;
        return high_bits == 0 || high_bits == 0xFFFF;
    };

    // Alignment check for physical and virtual addresses
    if (physical_address % PAGE_SIZE != 0)
    {
        debug_print("Physical address: %lx\n"
                    "Virtual address: %lx\n"
                    "RW: %hhx\n"
                    "NX: %hhx\n"
                    "SU: %hhx\n"
                    "PWT: %hhx\n"
                    "PCD: %hhx\n"
                    "PAT: %hhx\n"
                    "PK: %hhx\n"
                    "P: %hhx\n",
                    physical_address, virtual_address, is_read_write, disable_execution, is_supervisor,
                    page_write_through, page_cache_disable, page_attribute_table, protection_keys, is_present);

        debug_print("Error: Physical address not aligned to 4 KiB.\n");
        asm volatile("cli; hlt");
        return false;
    }

    if (virtual_address % PAGE_SIZE != 0)
    {
        debug_print("Error: Virtual address not aligned to 4 KiB. Given "
                    "address: %016lX\n",
                    virtual_address);
        asm volatile("cli; hlt");
        return false;
    }

    // Canonical check for the virtual address
    if (!is_canonical(virtual_address))
    {
        debug_print("Error: Virtual address is not canonical.\n");
        return false;
    }

    // Decompose the virtual address into its indexes
    auto indexes = split_virtual_address_to_structure(virtual_address);

    // Flags setup
    bool map_present = is_present;

    // PML4 setup
    auto pml4 = page_map;
    if (pml4[indexes.pml4].pdp_ppn == 0)
    {
        auto p = reinterpret_cast<uint64_t>(allocate_physical_page());
        if (!p)
        {
            debug_print("Error: Failed to allocate PDP.\n");
            return false;
        }
        memset(reinterpret_cast<void *>(p + get_higher_half_memory_offset()), 0, PAGE_SIZE);
        pml4[indexes.pml4].pdp_ppn = p >> 12;
    }

    // Apply flags to PML4E
    auto &pml4e = pml4[indexes.pml4];
    pml4e.p = map_present;
    pml4e.rw = is_read_write;
    pml4e.nx = disable_execution;
    pml4e.us = is_supervisor;

    // PDP setup
    auto *pdp = reinterpret_cast<struct pdp *>((pml4e.pdp_ppn << 12) + get_higher_half_memory_offset());
    if (pdp[indexes.pdp].pd_ppn == 0)
    {
        auto p = reinterpret_cast<uint64_t>(allocate_physical_page());
        if (!p)
        {
            debug_print("Error: Failed to allocate PD.\n");
            return false;
        }
        memset(reinterpret_cast<void *>(p + get_higher_half_memory_offset()), 0, PAGE_SIZE);
        pdp[indexes.pdp].pd_ppn = p >> 12;
    }

    // Apply flags to PDPE
    auto &pdpe = pdp[indexes.pdp];
    pdpe.p = map_present;
    pdpe.rw = is_read_write;
    pdpe.nx = disable_execution;
    pdpe.us = is_supervisor;

    // PD setup
    auto *pd = reinterpret_cast<struct pd *>((pdpe.pd_ppn << 12) + get_higher_half_memory_offset());
    if (pd[indexes.pd].pt_ppn == 0)
    {
        auto p = reinterpret_cast<uint64_t>(allocate_physical_page());
        if (!p)
        {
            debug_print("Error: Failed to allocate PT.\n");
            return false;
        }
        memset(reinterpret_cast<void *>(p + get_higher_half_memory_offset()), 0, PAGE_SIZE);
        pd[indexes.pd].pt_ppn = p >> 12;
    }

    // Apply flags to PDE
    auto &pde = pd[indexes.pd];
    pde.p = map_present;
    pde.rw = is_read_write;
    pde.nx = disable_execution;
    pde.us = is_supervisor;

    // PT setup
    auto *pt = reinterpret_cast<struct pt *>((pde.pt_ppn << 12) + get_higher_half_memory_offset());
    pt[indexes.pt].phys_ppn = physical_address >> 12;

    // Apply flags to PTE
    auto &pte = pt[indexes.pt];
    pte.p = map_present;
    pte.rw = is_read_write;
    pte.nx = disable_execution;
    pte.us = is_supervisor;
    pte.pwt = page_write_through;
    pte.pcd = page_cache_disable;
    pte.pat = page_attribute_table;
    pte.pk = protection_keys;

    // Invalidate TLB for the virtual address
    invalidate_page(virtual_address);

    return true;
}

void print_page_info(const uint64_t virtual_address)
{
    if ((virtual_address & 0xffff000000000000) != 0xffff000000000000 && (virtual_address & 0xffff000000000000) != 0)
    {
        debug_print("Invalid canonical virtual address: %016lX\n", virtual_address);
        return;
    }

    auto pml4 = static_cast<struct pml4 *>(get_current_page_map_address());
    auto indexes = split_virtual_address_to_structure(virtual_address);

    // Check PML4 entry
    auto &pml4_entry = pml4[indexes.pml4];
    if (!pml4_entry.p)
    {
        debug_print("PML4 entry not present for address: %016lX\n", virtual_address);
        return;
    }
    debug_print("PML4[%lu]: Present=%d RW=%d US=%d PWT=%d PCD=%d NX=%d PDP_PPN=%016lX\n", indexes.pml4, pml4_entry.p,
                pml4_entry.rw, pml4_entry.us, pml4_entry.pwt, pml4_entry.pcd, pml4_entry.nx, pml4_entry.pdp_ppn);

    // Get PDP entry
    auto *pdp = reinterpret_cast<struct pdp *>((pml4_entry.pdp_ppn << 12) + get_higher_half_memory_offset());
    auto &pdp_entry = pdp[indexes.pdp];
    if (!pdp_entry.p)
    {
        debug_print("PDP entry not present for address: %016lX\n", virtual_address);
        return;
    }
    debug_print("PDP[%lu]: Present=%d RW=%d US=%d PWT=%d PCD=%d NX=%d PD_PPN=%016lX\n", indexes.pdp, pdp_entry.p,
                pdp_entry.rw, pdp_entry.us, pdp_entry.pwt, pdp_entry.pcd, pdp_entry.nx, pdp_entry.pd_ppn);

    // Get PD entry
    auto *pd = reinterpret_cast<struct pd *>((pdp_entry.pd_ppn << 12) + get_higher_half_memory_offset());
    auto &pd_entry = pd[indexes.pd];
    if (!pd_entry.p)
    {
        debug_print("PD entry not present for address: %016lX\n", virtual_address);
        return;
    }
    debug_print("PD[%lu]: Present=%d RW=%d US=%d PWT=%d PCD=%d NX=%d PT_PPN=%016lX\n", indexes.pd, pd_entry.p,
                pd_entry.rw, pd_entry.us, pd_entry.pwt, pd_entry.pcd, pd_entry.nx, pd_entry.pt_ppn);

    // Get PT entry
    auto *pt = reinterpret_cast<struct pt *>((pd_entry.pt_ppn << 12) + get_higher_half_memory_offset());
    auto &pt_entry = pt[indexes.pt];
    if (!pt_entry.p)
    {
        debug_print("PT entry not present for address: %016lX\n", virtual_address);
        return;
    }
    debug_print("PT[%lu]: Present=%d RW=%d US=%d PWT=%d PCD=%d PAT=%d NX=%d "
                "Phys_PPN=%016lX\n",
                indexes.pt, pt_entry.p, pt_entry.rw, pt_entry.us, pt_entry.pwt, pt_entry.pcd, pt_entry.pat, pt_entry.nx,
                pt_entry.phys_ppn);

    // Calculate physical address
    const uint64_t physical_address = (pt_entry.phys_ppn << 12);
    debug_print("Mapped Physical Address: %016lX\n", physical_address);
}

uint64_t allocate_virtual_memory_kernel(uint64_t size)
{
    size = (size + PAGE_SIZE) & ~4095;

    if (remaining_bytes - size <= 0)
    {
        debug_print("No virtual memory left in the kernel. Halting!\n");
        asm volatile("cli; hlt");
    }

    auto address = base_address;
    base_address += size;
    return address | 0xffff000000000000;
}