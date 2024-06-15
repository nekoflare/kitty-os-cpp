//
// Created by Piotr on 04.06.2024.
//

#include "vmwsa.hpp"

static driver_handle_t vmwsa_entry(pci_dev* dev)
{
    kstd::printf("Initializing VMWSA device.\n");
    return {};
}

static driver_status_t vmwsa_ioctl(driver_handle_t* handle, uint64_t question, const char* buffer, char* answer)
{
    return DS_SUCCESS;
}

static driver_status_t vmwsa_cleanup()
{
    return DS_SUCCESS;
}

__attribute__((aligned(0x10))) static pci_requirements_t vmwsa_pci_requirement[] ={
        {
            .vendor_id = 0,
            .device_id = 0,
            ._class = 0x01,
            .subclass = 0x6,
            .prog_if = 0x1,
            .match_requirement = PCI_REQ_CSP
        },
        {
                .vendor_id = 0,
                .device_id = 0,
                ._class = 0x01,
                .subclass = 0x6,
                .prog_if = 0x2,
                .match_requirement = PCI_REQ_CSP
        },
        {
                .vendor_id = 0,
                .device_id = 0,
                ._class = 0x01,
                .subclass = 0x6,
                .prog_if = 0x0,
                .match_requirement = PCI_REQ_CSP
        }
};

driver_type driver_entry_t vmwsa_desc = {
        .driver_name = "SATA AHCI driver",
        .driver_author = "Piotr",
        .driver_description = "SATA AHCI driver",
        .driver_version = "Pre-release 1.00",
        .driver_designation = DT_STORAGE,
        .driver_load = DL_LOAD_ON_PCI_MATCH,
        .driver_entry = &vmwsa_entry,
        .driver_cleanup = &vmwsa_cleanup,
        .driver_ioctl = &vmwsa_ioctl,
        .requirements = reinterpret_cast<pci_requirements_t*>(&vmwsa_pci_requirement),
        .requirements_count = 3,
        .is_loaded = false
};