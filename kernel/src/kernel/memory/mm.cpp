//
// Created by Piotr on 05.05.2024.
//

#include "mm.hpp"

static bool mm_ready = false;

size_t mm_memmap_entry_count = 0;
limine_memmap_entry** mm_memmap_entries = nullptr;
limine_memmap_response* mm_memmap_response = nullptr;
limine_memmap_request mm_memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST,
        .revision = 0,
        .response = nullptr,
};

limine_hhdm_request mm_hhdm_request = {
        .id = LIMINE_HHDM_REQUEST,
        .revision = 0,
        .response = nullptr
};
limine_hhdm_response* mm_hhdm = nullptr;

size_t mm_memory_overall = 0;
size_t mm_memory_unusable = 0;
size_t mm_memory_usable = 0;
size_t mm_memory_reserved = 0;

uint8_t* mm_pmm_bitmap = nullptr;
size_t mm_pmm_bitmap_size = 0;
MemoryBitmap* mm_pmm_bitmap_controller = nullptr;

size_t MemoryBitmap::CalculatePageIdx(uint64_t addr)
{
    size_t page_idx = addr / VM_PAGE_SIZE;

    if (addr % VM_PAGE_SIZE != 0)
    {
        --page_idx;
    }

    return page_idx;
}

void MemoryBitmap::MarkAddressUsed(uint64_t addr)
{
    size_t page_idx = this->CalculatePageIdx(addr);

    this->Set(page_idx);
}

void MemoryBitmap::MarkAddressUnused(uint64_t addr)
{
    size_t page_idx = this->CalculatePageIdx(addr);

    this->Clear(page_idx);
}

void MemoryBitmap::MarkMemoryInRange(uint64_t start_addr, uint64_t end_addr)
{
    for (uint64_t addr = start_addr; addr <= end_addr; addr += VM_PAGE_SIZE)
    {
        this->MarkAddressUsed(addr);
    }
}

void MemoryBitmap::UnmarkMemoryInRange(uint64_t start_addr, uint64_t end_addr)
{
    for (uint64_t addr = start_addr; addr <= end_addr; addr += VM_PAGE_SIZE) {
        this->MarkAddressUnused(addr);
    }
}

uint64_t mm_physical_to_virtual_addr(uint64_t phys_addr)
{
    return mm_hhdm->offset + phys_addr;
}

void pmsa_initialize()
{
    if (mm_ready)
    {
        // Memory manager is already initialized. Return.
        return;
    }

    if (mm_memmap_request.response == nullptr)
    {
        // Memory manager isn't ready and mm_ready is already set to false.
        return;
    }

    if (mm_hhdm_request.response == nullptr)
    {
        // Same thing. It's a problem.
        return;
    }

    mm_hhdm = mm_hhdm_request.response;

    mm_memmap_response = mm_memmap_request.response;
    mm_memmap_entries = mm_memmap_response->entries;
    mm_memmap_entry_count = mm_memmap_response->entry_count;

    // Calculate how much memory we have installed
    for (size_t i = 0; mm_memmap_entry_count > i; i++)
    {
        auto entry = mm_memmap_entries[i];
        auto length = entry->length;
        auto type = entry->type;

        mm_memory_overall += length;

        switch (type)
        {
            case LIMINE_MEMMAP_USABLE:
                mm_memory_usable += length;
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                mm_memory_reserved += length;
                break;
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
                mm_memory_reserved += length;
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                mm_memory_reserved += length;
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                mm_memory_unusable += length;
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                mm_memory_reserved += length;
                break;
            case LIMINE_MEMMAP_RESERVED:
                mm_memory_reserved += length;
                break;
            case LIMINE_MEMMAP_ACPI_NVS:
                mm_memory_reserved += length;
                break;
            default:
                kstd::printf("[MM] Invalid entry type. Entry index: %ld.\n", i);
                mm_memory_unusable += length;
                break;
        }
    }

    double usable_memory_percentage = (static_cast<double>(mm_memory_usable) / mm_memory_overall) * 100.0;

    kstd::printf("[MM] Memory: %f / %f [MB]\n[MM] %f%% of memory is usable.\n", static_cast<double>(mm_memory_usable) / 1024 / 1024, static_cast<double>(mm_memory_overall) / 1024 / 1024, usable_memory_percentage);

    // This time I thought of something better.
    // Find the biggest base and add length to it in all entries.
    // The biggest value will be the end address of memory.
    size_t largest_address = 0;
    for (size_t i = 0; mm_memmap_entry_count > i; i++)
    {
        auto entry = mm_memmap_entries[i];
        auto base = entry->base;
        auto length = entry->length;

        if (base + length > largest_address)
        {
            largest_address = base + length;
        }
    }

    kstd::printf("[MM] Largest address: %lx\n", largest_address);

    largest_address = mm_align_mem(largest_address, VM_PAGE_SIZE);
    size_t bitmap_required_size = largest_address / VM_PAGE_SIZE;

    size_t bitmap_bytes = bitmap_required_size / 8;
    mm_pmm_bitmap_size = bitmap_bytes;

    if (bitmap_required_size % 8 != 0)
    {
        bitmap_bytes ++;
    }

    kstd::printf("[MM] Bytes required for the bitmap: %lx\n", bitmap_required_size);
    kstd::printf("[MM] Boundaries of memory 0 - %ld (MB)\n", largest_address / 1024 / 1024);

    // Find space for the bitmap.
    for (size_t i = 0; mm_memmap_entry_count > i; i++)
    {
        auto& entry = mm_memmap_entries[i];
        auto& base = entry->base;
        auto& length = entry->length;
        auto type = entry->type;

        if (length >= bitmap_bytes && type == LIMINE_MEMMAP_USABLE)
        {
            kstd::printf("[MM] Found the suitable entry! Base: %lx Length: %lx Idx: %lx\n", base, length, i);

            mm_pmm_bitmap = reinterpret_cast<uint8_t*>(base);
            base += bitmap_bytes;
            length -= bitmap_bytes;

            break;
        }
    }

    kstd::printf("[MM] Recalculating memory...\n");
    mm_memory_usable = 0;
    for (size_t i = 0; mm_memmap_entry_count > i; i++)
    {
        auto entry = mm_memmap_entries[i];
        auto length = entry->length;
        auto type = entry->type;

        if (type == LIMINE_MEMMAP_USABLE)
        {
            mm_memory_usable += length;
        }
    }

    usable_memory_percentage = (static_cast<double>(mm_memory_usable) / mm_memory_overall) * 100.0;

    kstd::printf("[MM] Memory: %f / %f [MB]\n[MM] %f%% of memory is usable.\n", static_cast<double>(mm_memory_usable) / 1024 / 1024, static_cast<double>(mm_memory_overall) / 1024 / 1024, usable_memory_percentage);

    // Clear the bitmap.
    kstd::memset(mm_pmm_bitmap, 0xff, mm_pmm_bitmap_size);

    static MemoryBitmap mm_pmm_bitmap_ctrl;

    // Convert physical address of mm_pmm_bitmap to virtual address
    uint64_t virtual_bitmap_addr = mm_physical_to_virtual_addr(reinterpret_cast<uint64_t>(mm_pmm_bitmap));

    kstd::printf("bitmap addr: %lx\n", virtual_bitmap_addr);
    kstd::printf("hhdm   addr: %lx\n", mm_hhdm->offset);

    // Initialize the control bitmap with the virtual address and size
    mm_pmm_bitmap_ctrl.Initialize(reinterpret_cast<uint8_t*>(virtual_bitmap_addr), mm_pmm_bitmap_size);
    mm_pmm_bitmap_controller = &mm_pmm_bitmap_ctrl;

    mm_pmm_bitmap_controller->ClearToOnes();

    for (size_t i = 0; mm_memmap_entry_count > i; i++)
    {
        auto entry = mm_memmap_entries[i];
        auto base = entry->base;
        auto length = entry->length;
        auto type = entry->type;

        if (type == LIMINE_MEMMAP_USABLE)
        {
            mm_pmm_bitmap_controller->UnmarkMemoryInRange(base, base + length);
        }
    }

    kstd::printf("Free pages: %ld\n", mm_memory_usable / VM_PAGE_SIZE);

    mm_ready = true;
}

uint64_t mm_align_mem(uint64_t addr, uint64_t align)
{
    uint64_t remainder = addr % align;

    if (remainder != 0)
    {
        uint64_t adjustment = align - remainder;

        addr += adjustment;
    }

    return addr;
}

bool pmsa_is_ready()
{
    return mm_ready;
}

const char* mm_entry_type_to_string(uint64_t type)
{
    switch (type)
    {
        case LIMINE_MEMMAP_USABLE:
            return "Usable memory";
        case LIMINE_MEMMAP_RESERVED:
            return "Reserved memory.";
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            return "ACPI Reclaimable";
        case LIMINE_MEMMAP_ACPI_NVS:
            return "ACPI Non-Volatile Storage (Reserved)";
        case LIMINE_MEMMAP_BAD_MEMORY:
            return "Bad memory";
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            return "Bootloader (Reclaimable memory)";
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            return "Kernel and modules (Reserved)";
        case LIMINE_MEMMAP_FRAMEBUFFER:
            return "Framebuffer (Reserved)";
        default:
            return "Invalid entry type.";
    }
}

void mm_enumerate_memmap_entries(bool compact_write)
{
    if (!pmsa_is_ready())
        return;

    kstd::printf("The whole memory map on this machine:\n");

    for (size_t i = 0; mm_memmap_entry_count > i; i++)
    {
        auto entry = mm_memmap_entries[i];
        auto base = entry->base;
        auto length = entry->length;
        auto type = entry->type;

        auto type_name = mm_entry_type_to_string(type);

        if (!compact_write)
            kstd::printf("Entry %ld:    Base: %lx    \nLength: %lx\n    Type: %s\n", i + 1, base, length, type_name);
        else
            kstd::printf("E%ld: Base: %lx Length: %lx Type: %s\n", i+1, base, length, type_name);
    }
}

void* pmsa_alloc_page()
{
    // ask memory controller for the page.
    size_t page_idx = mm_pmm_bitmap_controller->FindFirstCleared();

    // Check if page_idx is good
    if (page_idx == SIZE_MAX)
    {
        return nullptr;
    }

    // Convert page_idx to the address (physical)
    uint64_t physical_address = page_idx * VM_PAGE_SIZE;

    // Mark the address as used.
    mm_pmm_bitmap_controller->MarkAddressUsed(physical_address);

    kstd::printf("Allocated a page. Address: %lx\n", physical_address);

    // Return the address
    return reinterpret_cast<void*>(physical_address);
}

void pmsa_free_page(void* ptr)
{
    // Convert the pointer in to physical one
    uint64_t physical_address = reinterpret_cast<uint64_t>(ptr);

    kstd::printf("Freed a page. Address: %lx\n", physical_address);

    mm_pmm_bitmap_controller->MarkAddressUnused(physical_address);
}

uint64_t get_logical_address_pml4()
{
    uint64_t cr3 = read_cr3();
    uint64_t pdbr = cr3 & ~0xfff;
    pdbr += mm_hhdm->offset;
    return pdbr;
}

VirtAddress mm_get_va_components(void* virtual_address)
{
    VirtAddress components;
    uint64_t addr = (uint64_t)virtual_address;
    components.ones_or_zeroes = (addr >> 48) & 0xFFFF;
    components.pml4e = (addr >> 39) & 0x1FF;
    components.pdpe = (addr >> 30) & 0x1FF;
    components.pde = (addr >> 21) & 0x1FF;
    components.pte = (addr >> 12) & 0x1FF;
    components.offset = addr & 0xFFF;
    return components;
}

VirtAddress mm_get_va_components(uint64_t virtual_address)
{
    VirtAddress components;
    uint64_t addr = virtual_address;
    components.ones_or_zeroes = (addr >> 48) & 0xFFFF;
    components.pml4e = (addr >> 39) & 0x1FF;
    components.pdpe = (addr >> 30) & 0x1FF;
    components.pde = (addr >> 21) & 0x1FF;
    components.pte = (addr >> 12) & 0x1FF;
    components.offset = addr & 0xFFF;
    return components;
}

/*
     * Assume:
     * pml4e* pml4e is a virtual address pointing to PML4E table.
     * uint64_t virt_address is a maybe invalid virtual address. (We won't correct it)
     * uint64_t phys_address is a maybe unaligned physical address. (We won't align it)
     * int prot_flags can be invalid. We won't fix it, we won't tell you.
     * int map_flags can be invalid. We won't fix it, we won't tell you.
*/
bool vmsa_map_page(
        pml4e* pml4e,
        uint64_t virt_address,
        uint64_t phys_address,
        int prot_flags,
        int map_flags,
        int misc_flags,
        uint64_t pke
)
{
    if (pml4e == nullptr)
    {
        return MAP_FAILURE; // Couldn't be processed
    }

    // Check for GPFs.
    uint64_t tmp_va = virt_address & ((uint64_t)0xffff << 48);
    tmp_va >>= 48;

    if (tmp_va != 0xffff && tmp_va != 0x0000)
    {
            kstd::printf("The VA has first 16-bits not all ones or zeroes. Will cause a GPF.\n");
            return MAP_FAILURE;
    }

    // Check alignments
    // Alignment of VA.
    if (virt_address % VM_PAGE_SIZE != 0)
    {
        kstd::printf("The VA is unaligned to 4096!\n");
        return MAP_FAILURE;
    }

    // Alignment of PA.
    if (phys_address % VM_PAGE_SIZE != 0)
    {
        kstd::printf("The PA is unaligned to 4096!\n");
        return MAP_FAILURE;
    }

    // All OK!
    kstd::printf("Mapping %lx -> %lx.\n", phys_address, virt_address);

    // Split a VA in to specific values.
    VirtAddress va_items = mm_get_va_components(virt_address);
    uint64_t pml4e_idx, pdpe_idx, pde_idx, pte_idx;

    pml4e_idx = va_items.pml4e;
    pdpe_idx = va_items.pdpe;
    pde_idx = va_items.pde;
    pte_idx = va_items.pte;

    // Time to check PML4E if it has entry pointing to something already.

    if (pml4e[pml4e_idx].pdpe_base_address == 0)
    {
        kstd::printf("PML4E entry isn't pointing anywhere. Not implemented.\n");

        return MAP_FAILURE;
    }

    // Get the address of PDPE entry.
    uint64_t pdpe_phys = pml4e[pml4e_idx].pdpe_base_address << 12;
    struct pdpe* pdpe = reinterpret_cast<struct pdpe*>(pdpe_phys + mm_hhdm->offset);

    if (pdpe[pdpe_idx].pde_base_address == 0)
    {
        kstd::printf("PDPE entry isn't pointing anywhere. Not implemented.\n");

        return MAP_FAILURE;
    }

    uint64_t pde_phys = pdpe[pdpe_idx].pde_base_address << 12;
    struct pde* pde = reinterpret_cast<struct pde*>(pde_phys + mm_hhdm->offset);

    if (pde[pde_idx].pte_base_address == 0)
    {
        kstd::printf("PDE entry isn't pointing anywhere. Not implemented.\n");

        return MAP_FAILURE;
    }

    uint64_t pte_phys = pde[pde_idx].pte_base_address << 12;
    struct pte* pte = reinterpret_cast<struct pte*>(pte_phys + mm_hhdm->offset);

    bool force_map = (prot_flags & PROT_FORCE_MAP) > 0 ? 1 : 0;

    if (pte[pte_idx].pp_base_address != 0 && !force_map)
    {
        kstd::printf("There's something already mapped.\n");

        return MAP_FAILURE;
    }

    // Map.
    bool prot_rw = (prot_flags & PROT_RW) > 0 ? 1 : 0;
    bool prot_exec = (prot_flags & PROT_EXEC) > 0 ? 1 : 0;
    bool prot_su = (prot_flags & PROT_SUPERVISOR) > 0 ? 1 : 0;

    kstd::printf("Mapping with permissions of: ");
    if (prot_rw) kstd::printf("RW ");
    if (!prot_exec) kstd::printf("NX ");
    if (prot_su) kstd::printf("SU ");
    kstd::printf("\n");

    bool map_pres = (map_flags & MAP_PRESENT) > 0 ? 1 : 0;
    bool map_pwt = (map_flags & MAP_PWT) > 0 ? 1 : 0;
    bool map_pcd = (map_flags & MAP_PCD) > 0 ? 1 : 0;
    bool map_pat = (map_flags & MAP_PAT) > 0 ? 1 : 0;
    bool map_g = (map_flags & MAP_GLOBAL) > 0 ? 1 : 0;

    kstd::printf("And mapping flags of: ");
    if (map_pres) kstd::printf("P ");
    if (map_pwt) kstd::printf("PWT ");
    if (map_pcd) kstd::printf("PCD ");
    if (map_pat) kstd::printf("PAT ");
    if (map_g) kstd::printf("G ");
    kstd::printf("\n");

    pte[pte_idx].pp_base_address = phys_address >> 12;
    pte[pte_idx].p = map_pres;
    pte[pte_idx].rw = prot_rw;
    pte[pte_idx].us = prot_su;
    pte[pte_idx].pwt = map_pwt;
    pte[pte_idx].pcd = map_pcd;
    pte[pte_idx].a = 0;
    pte[pte_idx].d = 0;
    pte[pte_idx].pat = map_pat;
    pte[pte_idx].g = map_g;
    pte[pte_idx].avl = 0;
    pte[pte_idx].available = 0;
    pte[pte_idx].pke = pke;
    pte[pte_idx].nx = prot_exec;

    // invalidate the table if told to do so.
    if ((misc_flags & MISC_INVLPG) > 0)
    {
        mm_invlpg(reinterpret_cast<void*>(virt_address));
        kstd::printf("Invalidated the page as told.\n");
    }

    return MAP_SUCCESS;
}

bool vmsa_map_pages(
        pml4e* pml4e,
        uint64_t virt_address,
        uint64_t phys_address,
        uint64_t page_count,
        int prot_flags,
        int map_flags,
        int misc_flags,
        uint64_t pke
)
{
    for (uint64_t i = 0; page_count > i; i++)
    {
        if (!vmsa_map_page(pml4e, virt_address + (VM_PAGE_SIZE * i), phys_address + (VM_PAGE_SIZE * i), prot_flags, map_flags, misc_flags, pke)) return MAP_FAILURE;
    }

    return MAP_SUCCESS;
}

void pmsa_test()
{
    kstd::printf("[MM] PDBR: %lx\n", get_logical_address_pml4());

    pml4e* _Pml4e = reinterpret_cast<pml4e*>(get_logical_address_pml4());
    VirtAddress va = mm_get_va_components(reinterpret_cast<void*>(&pmsa_test));

    kstd::printf("PML4E: %p\n", static_cast<void*>(_Pml4e));

    pdpe* _Pdpe = reinterpret_cast<pdpe*>(_Pml4e[va.pml4e].pdpe_base_address << 12);

    kstd::printf("PDPE: %p\n", static_cast<void*>(_Pdpe));

    pde* _Pde = reinterpret_cast<pde*>(_Pdpe[va.pdpe].pde_base_address << 12);

    kstd::printf("PDE: %p\n", static_cast<void*>(_Pde));

    pte* _Pte = reinterpret_cast<pte*>(_Pde[va.pde].pte_base_address << 12);

    kstd::printf("PTE: %p\n", static_cast<void*>(_Pte));

    pte* _PteVirt = reinterpret_cast<pte*>(reinterpret_cast<uint64_t>(_Pte) + mm_hhdm->offset);

    kstd::printf("Phys address: %lx\n", _PteVirt[va.pte].pp_base_address << 12);

    return;
}

bool vmsa_cmp_pml4(
        pml4e* pml4e,
        size_t index,
        int prot_flags,
        int map_flags
)
{
    return (pml4e[index].rw == ((prot_flags & PROT_RW) > 0)) &&
    (pml4e[index].p == ((map_flags & MAP_PRESENT) > 0)) &&
    (pml4e[index].us == ((prot_flags & PROT_SUPERVISOR) > 0)) &&
    (pml4e[index].pwt == ((map_flags & MAP_PWT) > 0)) &&
    (pml4e[index].pcd == ((map_flags & MAP_PCD) > 0)) &&
    (pml4e[index].nx == ((prot_flags & PROT_EXEC) > 0));
}

bool vmsa_cmp_pdpe(
        pdpe* pdpe,
        size_t index,
        int prot_flags,
        int map_flags
)
{
    return (pdpe[index].p == ((map_flags & MAP_PRESENT) > 0)) &&
    (pdpe[index].rw == ((prot_flags & PROT_RW) > 0)) &&
    (pdpe[index].us == ((prot_flags & PROT_SUPERVISOR) > 0)) &&
    (pdpe[index].pwt == ((map_flags & MAP_PWT) > 0)) &&
    (pdpe[index].pcd == ((map_flags & MAP_PCD) > 0)) &&
    (pdpe[index].nx == ((prot_flags & PROT_EXEC) > 0));
}

bool vmsa_cmp_pde(
        pde* pde,
        size_t index,
        int prot_flags,
        int map_flags
)
{
    return (pde[index].p == ((map_flags & MAP_PRESENT) > 0)) &&
    (pde[index].rw == ((prot_flags & PROT_RW) > 0)) &&
    (pde[index].us == ((prot_flags & PROT_SUPERVISOR) > 0)) &&
    (pde[index].pwt == ((map_flags & MAP_PWT) > 0)) &&
    (pde[index].pcd == ((map_flags & MAP_PCD) > 0)) &&
    (pde[index].nx == ((prot_flags & PROT_EXEC) > 0)) &&
    (pde[index].g == ((map_flags & MAP_GLOBAL) > 0));
}

bool vmsa_cmp_pte(
        pte* pte,
        size_t index,
        int prot_flags,
        int map_flags
)
{
    return (pte[index].p == ((map_flags & MAP_PRESENT) > 0)) &&
    (pte[index].rw == ((prot_flags & PROT_RW) > 0)) &&
    (pte[index].us == ((prot_flags & PROT_SUPERVISOR) > 0)) &&
    (pte[index].pwt == ((map_flags & MAP_PWT) > 0)) &&
    (pte[index].pcd == ((map_flags & MAP_PCD) > 0)) &&
    (pte[index].nx == ((prot_flags & PROT_EXEC) > 0)) &&
    (pte[index].g == ((map_flags & MAP_GLOBAL) > 0));
}

uint64_t vmsa_alloc(pml4e* pml4e, int prot_flags, int map_flags, int misc_flags, bool use_kernel)
{
    return 0;
}