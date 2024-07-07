//
// Created by Piotr on 18.05.2024.
//

#include "vmm.hpp"

limine_hhdm_response* vmm_hhdm = nullptr;
limine_hhdm_request vmm_hhdm_request = {
        .id = LIMINE_HHDM_REQUEST,
        .revision = 0,
        .response = nullptr
};

void vmm_init()
{
    [[gnu::used]] static bool run_once = [](){
        if (vmm_hhdm_request.response == nullptr)
        {
            if constexpr (vmm_verbose)
            {
                kstd::printf("[VMM] HDDM request response in null.\n");
                asm volatile ("cli; hlt");
            }
        }

        vmm_hhdm = vmm_hhdm_request.response;

        uint64_t this_pml4e = vmm_get_pml4();
        kstd::printf("PML4e address: %lx\n", this_pml4e);

        return true;
    }();
}

bool vmm_map(pml4e* pml4e, uint64_t virt_address, uint64_t phys_address, int prot_flags, int map_flags, int misc_flags)
{
    vmm_address va = vmm_split_va(virt_address);

    // Alignment check.
    if (phys_address % PAGE_SIZE != 0)
    {
        if constexpr (vmm_verbose)
        {
            kstd::printf("Physical address isn't aligned to page.\n");
        }

        unreachable();
        return false;
    }

    if (virt_address % PAGE_SIZE != 0)
    {
        if constexpr (vmm_verbose)
        {
            kstd::printf("Virtual address isn't aligned to the page.\n");
        }
        unreachable();
    }

    // Get separate flags already.
    bool protection_rw = (prot_flags & PROT_RW) > 0 ? true : false;
    bool protection_noexec = (prot_flags & PROT_NOEXEC) > 0 ?  true : false;
    bool protection_supervisor = (prot_flags & PROT_SUPERVISOR) > 0 ?  true : false;

    bool map_present = (map_flags & MAP_PRESENT) > 0 ?  true : false;
    bool map_global = (map_flags & MAP_GLOBAL) > 0 ?  true : false;

    if (!protection_rw) kstd::printf("WARNING! NO RW FLAG SET.\n");
    if (!map_present) kstd::printf("WARNING! NO PRESENT FLAG SET.\n");

    bool misc_invlpg = (misc_flags & MISC_INVLPG) > 0 ?  true : false;

    if (pml4e[va.pml4e].pdpe_ptr == 0) {
        uint64_t p = pmm_alloc_page();

        kstd::memset(vmm_make_virtual<uint8_t*>(p), 0, 4096);

        pml4e[va.pml4e].pdpe_ptr = p >> 12;
        pml4e[va.pml4e].present = map_present;
        pml4e[va.pml4e].read_write = protection_rw;

        if constexpr (vmm_verbose)
                kstd::printf("PDPE ptr: %lx\n", p);
    }

    pdpe* pdpe = reinterpret_cast<struct pdpe*>((pml4e[va.pml4e].pdpe_ptr << 12) + vmm_hhdm->offset);
    if (pdpe[va.pdpe].pde_ptr == 0) {
        uint64_t p = pmm_alloc_page();

        kstd::memset(vmm_make_virtual<uint8_t*>(p), 0, 4096);

        pdpe[va.pdpe].pde_ptr = p >> 12;
        pdpe[va.pdpe].present = map_present;
        pdpe[va.pdpe].read_write = protection_rw;

        if constexpr (vmm_verbose)
            kstd::printf("PDE ptr: %lx\n", p);
    }

    pde* pde =  reinterpret_cast<struct pde*>((pdpe[va.pdpe].pde_ptr << 12) + vmm_hhdm->offset);
    if (pde[va.pde].pte_ptr == 0) {
        uint64_t p = pmm_alloc_page();

        kstd::memset(vmm_make_virtual<uint8_t*>(p), 0, 4096);

        pde[va.pde].pte_ptr = p >> 12;
        pde[va.pde].present = map_present;
        pde[va.pde].read_write = protection_rw;

if constexpr (vmm_verbose)
        kstd::printf("PTE ptr: %lx\n", p);

    }

    pte* pte = reinterpret_cast<struct pte*>((pde[va.pde].pte_ptr << 12) + vmm_hhdm->offset);

    // Apply flags to everything that we know
    pml4e[va.pml4e].present = map_present;
    pml4e[va.pml4e].read_write = protection_rw; // Set read/write flag in PML4E

    pdpe[va.pdpe].present = map_present;
    pdpe[va.pdpe].read_write = protection_rw; // Set read/write flag in PDP

    pde[va.pde].present = map_present;
    pde[va.pde].read_write = protection_rw; // Set read/write flag in PDE

    pte[va.pte].present = map_present;
    pte[va.pte].read_write = protection_rw;
    pte[va.pte].user_supervisor = !protection_supervisor; // Set user/supervisor flag in PTE
    pte[va.pte].no_execute = protection_noexec; // Set no execute flag in PTE
    pte[va.pte].global = map_global;
    pte[va.pte].phys_ptr = phys_address >> 12;

    if (misc_invlpg) {
        //kstd::printf("Invalidated.\n");
        flush_tlb(virt_address);
    }

    return true;
}

// On dear god, don't use this function.
template <typename T>
void vmm_print_flags(T v) {
    if (v.present) kstd::printf("Present, ");
    if (v.read_write) kstd::printf("Read/Write, ");
    if (v.user_supervisor) kstd::printf("User, ");
    else kstd::printf("Supervisor, ");
    if (v.page_write_through) kstd::printf("Page Write Through, ");
    if (v.page_cache_disable) kstd::printf("Page Cache Disable, ");
    if (v.no_execute) kstd::printf("No Execute, ");

    if constexpr (std::is_same_v<T, pte>)
    {
        if (v.page_attribute_table) kstd::printf("Page Attribute Table, ");
        if (v.global) kstd::printf("Global, ");
    }

    kstd::printf("\n");
}
void vmm_test(void* addr)
{
    kstd::printf("[VMM] Getting physical address of addr %p.\n", addr);

    auto* pml4e = reinterpret_cast<struct pml4e*>(vmm_get_pml4());
    pml4e = vmm_make_virtual_singular(pml4e);

    vmm_address va = vmm_split_va(addr);

    kstd::printf("[VMM] PML4e address: %p.\n", reinterpret_cast<void*>(pml4e));

    if (va.padding != 0 && va.padding != 0xffff)
    {
        kstd::printf("[VMM] The padding isn't equal to 0x0000 or 0xffff which **will** cause a GPF when doing memory operation with it.\n");
    }

    pdpe* pdpe = reinterpret_cast<struct pdpe*>(pml4e[va.pml4e].pdpe_ptr << 12);
    pdpe = vmm_make_virtual_singular(pdpe);

    if (!pml4e[va.pml4e].present)
    {
        kstd::printf("[VMM] PML4e entry pointing to PDPe isn't present.\n");

        return;
    }
    if (pdpe == nullptr) {
        kstd::printf("[VMM] PML4e entry pointing to PDPe is present but is null.\n");

        return;
    }

    kstd::printf("[VMM] PDPe address: %p\n", reinterpret_cast<void*>(pdpe));

    pde* pde = reinterpret_cast<struct pde*>(pdpe[va.pdpe].pde_ptr << 12);
    pde = vmm_make_virtual_singular(pde);

    if (!pdpe[va.pdpe].present)
    {
        kstd::printf("[VMM] PDPe entry pointing to PDe isn't present.\n");

        return;
    }

    if (pde == nullptr)
    {
        kstd::printf("[VMM] PDPe entry pointing to PDe is present but is null.\n");

        return;
    }

    kstd::printf("[VMM] PDe address: %p\n", reinterpret_cast<void*>(pde));

    pte* pte = reinterpret_cast<struct pte*>(pde[va.pde].pte_ptr << 12);
    pte = vmm_make_virtual_singular(pte);

    if (!pde[va.pde].present)
    {
        kstd::printf("[VMM] PDe entry pointing to PTe isn't present.\n");

        return;
    }

    if (pte == nullptr)
    {
        kstd::printf("[VMM] PDe entry pointing to PTe is present but null.\n");

        return;
    }

    kstd::printf("[VMM] PTe address: %p\n", reinterpret_cast<void*>(pte));

    kstd::printf("[VMM] Physical address: %lx\n", pte[va.pte].phys_ptr << 12);

    kstd::printf("Flags for PML4e: \n");
    vmm_print_flags(pml4e[va.pml4e]);

    kstd::printf("Flags for PDPe: \n");
    vmm_print_flags(pdpe[va.pdpe]);

    kstd::printf("Flags for PDe: \n");
    vmm_print_flags(pde[va.pde]);

    kstd::printf("Flags for PTe: \n");
    vmm_print_flags(pte[va.pte]);
}