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

static constexpr bool vmm_verbose = true;

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

        pml4e* this_pml4e = vmm_get_pml4();
        kstd::printf("PML4e address: %lx\n", this_pml4e);

        return true;
    }();
}


