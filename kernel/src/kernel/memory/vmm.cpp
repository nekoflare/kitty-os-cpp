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
    static bool run_once = [](){
        if (vmm_hhdm_request.response == nullptr)
        {
            if constexpr (vmm_verbose)
            {
                kstd::printf("[VMM] HDDM request response in null.\n");
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
    if (pml4e == nullptr) return false;

    vmm_address va = vmm_split_va(virt_address);

    // Alignment check.
    if (phys_address % PAGE_SIZE != 0)
    {
        if constexpr (vmm_verbose)
        {
            kstd::printf("Physical address isn't aligned to page.\n");
        }

        return false;
    }

    // Get separate flags already.
    bool protection_rw = (prot_flags & PROT_RW) > 0 ? true : false;
    bool protection_noexec = (prot_flags & PROT_NOEXEC) > 0 ?  true : false;
    bool protection_supervisor = (prot_flags & PROT_SUPERVISOR) > 0 ?  true : false;

    bool map_present = (map_flags & MAP_PRESENT) > 0 ?  true : false;
    bool map_global = (map_flags & MAP_GLOBAL) > 0 ?  true : false;

    bool misc_invlpg = (misc_flags & MISC_INVLPG) > 0 ?  true : false;
    bool misc_force_map = (misc_flags & MISC_FORCE) > 0 ? true : false;

    kstd::printf("Present: %hhx\n", map_present);
    kstd::printf("RW: %hhx\n", protection_rw);

    if (pml4e[va.pml4e].pdpe_ptr == 0) {
        uint64_t p = pmm_alloc_page();

        kstd::memset(vmm_make_virtual<uint8_t*>(p), 0, 4096);

        pml4e[va.pml4e].pdpe_ptr = p >> 12;
    }

    kstd::printf("PDPE ptr: %llx\n", pml4e[va.pml4e].pdpe_ptr << 12);

    pdpe* pdpe = reinterpret_cast<struct pdpe*>((pml4e[va.pml4e].pdpe_ptr << 12) + vmm_hhdm->offset);
    if (pdpe[va.pdpe].pde_ptr == 0) {
        uint64_t p = pmm_alloc_page();

        kstd::memset(vmm_make_virtual<uint8_t*>(p), 0, 4096);

        pdpe[va.pdpe].pde_ptr = p >> 12;
    }

    kstd::printf("PDE ptr: %llx\n", pdpe[va.pdpe].pde_ptr << 12);

    pde* pde =  reinterpret_cast<struct pde*>((pdpe[va.pdpe].pde_ptr << 12) + vmm_hhdm->offset);
    if (pde[va.pde].pte_ptr == 0) {
        uint64_t p = pmm_alloc_page();

        kstd::memset(vmm_make_virtual<uint8_t*>(p), 0, 4096);

        pde[va.pde].pte_ptr = p >> 12;
    }

    kstd::printf("PTE ptr: %llx\n", pde[va.pde].pte_ptr << 12);

    pte* pte =  reinterpret_cast<struct pte*>((pde[va.pde].pte_ptr << 12) + vmm_hhdm->offset);


    // Apply flags to everything that we know
    pml4e[va.pml4e].present = map_present;
    pml4e[va.pml4e].read_write = protection_rw; // Set read/write flag in PML4E
    pdpe[va.pdpe].present = map_present;
    pdpe[va.pdpe].read_write = protection_rw; // Set read/write flag in PDP
    pde[va.pde].present = map_present;
    pde[va.pde].read_write = protection_rw; // Set read/write flag in PDE
    pte[va.pte].present = map_present;
    pte[va.pte].read_write = protection_rw;
    pte[va.pte].user_supervisor = protection_supervisor; // Set user/supervisor flag in PTE
    pte[va.pte].no_execute = protection_noexec; // Set no execute flag in PTE
    pte[va.pte].global = map_global;
    pte[va.pte].phys_ptr = phys_address >> 12;

    if (!pml4e[va.pml4e].present) kstd::printf("PML4E ISNT PRESENT.\n");
    if (!pdpe[va.pdpe].present) kstd::printf("PDPE ISNT PRESENT.\n");
    if (!pde[va.pde].present) kstd::printf("PDE ISNT PRESENT.\n");
    if (!pte[va.pte].present) kstd::printf("PTE ISNT PRESENT.\n");

    kstd::printf("phys ptr: %llx", pte[va.pte].phys_ptr);

    if (misc_invlpg) {
        flush_tlb(virt_address);
    }

    return true;
}