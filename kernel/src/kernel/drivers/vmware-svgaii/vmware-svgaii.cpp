//
// Created by Piotr on 13.05.2024.
//

#include "vmware-svgaii.hpp"

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kdu/driver_entry.hpp>
#include <kstd/kstdio.hpp>

static driver_status_t vmware_svgaii_entry(pci_handle_t* pci_handle)
{
    kstd::printf("Loaded VMWare SVGA II driver!\n");

    if (pci_handle->vendor_id != 0x15ad ||
        pci_handle->device_id != 0x0405)
    {
        kstd::printf("Vendor ID or Device ID is invalid!\n");

        return DS_FAILURE;
    }


    return DS_SUCCESS;
}

static driver_status_t vmware_svgaii_cleanup()
{
    kstd::printf("[VMWARE SVGA II] Poof!\n");

    return DS_SUCCESS;
}

static driver_status_t vmware_svgaii_ioctl(
     driver_handle_t* driver_handle,
     driver_question_t driver_question,
     const char* question_buffer,
     char* ioctl_answer
)
{


    return DS_SUCCESS;
}

__attribute__((aligned(0x10))) static pci_requirements_t pci_requirements[] = {
{
    .vendor_id = 0x15ad,
    .device_id = 0x0405,
    .match_requirement = PCI_REQ_VD
    }
};

driver_type driver_entry_t vmware_svgaii_driver_descriptor = {
        .driver_name = "VMWare SVGA II driver.",
        .driver_author = "Piotr",
        .driver_description = "Basic graphics driver.",
        .driver_version = "Pre-release 1.00",
        .driver_designation = DT_GPU,
        .driver_load = DL_LOAD_ON_PCI_MATCH,
        .driver_entry = &vmware_svgaii_entry,
        .driver_cleanup = &vmware_svgaii_cleanup,
        .driver_ioctl = &vmware_svgaii_ioctl,
        .requirements = reinterpret_cast<pci_requirements_t*>(&pci_requirements),
        .requirements_count = 1,
        .is_loaded = false
};