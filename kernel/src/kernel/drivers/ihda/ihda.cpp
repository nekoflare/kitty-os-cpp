//
// Created by Piotr on 12.05.2024.
//

#include "ihda.hpp"
#include <control/control.hpp>

static driver_status_t ihda_driver_entry(pci_handle_t* pci_handle)
{
    kstd::printf("[IHDA] IHDA driver has been called.\n");

    return DS_SUCCESS;
}

static driver_status_t ihda_driver_cleanup()
{
    kstd::printf("[IHDA] IHDA driver has been cleaned up.\n");

    return DS_SUCCESS;
}

static driver_status_t ihda_ioctl(
        driver_handle_t* driver_handle,
        driver_question_t driver_question,
        const char* question_buffer,
        char* ioctl_answer
)
{
    return DS_SUCCESS;
}

static const char* ihda_driver_name = "Intel HD Audio driver";
static const char* ihda_driver_author = "Piotr 12.05.2024";
static const char* ihda_driver_description = "Basic audio driver for IHDA.";
static const char* ihda_version = "Pre-release 1.0.0";

__attribute__((aligned(0x10))) static pci_requirements_t pci_requirements[] = {
        {
            .vendor_id = 0x8086,
            .device_id = 0x2668,
            .match_requirement = PCI_REQ_VD
        },
        {
            .vendor_id = 0x8086,
            .device_id = 0x27D8,
            .match_requirement = PCI_REQ_VD
        },
        {
            .vendor_id = 0x1002,
            .device_id = 0x4383,
            .match_requirement = PCI_REQ_VD
        },
        {
            ._class = 0x4,
            .subclass = 0x3,
            .prog_if = 0x0,
            .match_requirement = PCI_REQ_CSP
        }
};

driver_type driver_entry_t ihda_driver_descriptor = {
        .driver_name = ihda_driver_name,
        .driver_author = ihda_driver_author,
        .driver_description = ihda_driver_description,
        .driver_version = ihda_version,
        .driver_designation = DT_AUDIO,
        .driver_load = DL_LOAD_ON_PCI_MATCH,
        .driver_entry = &ihda_driver_entry,
        .driver_cleanup = &ihda_driver_cleanup,
        .driver_ioctl = &ihda_ioctl,
        .requirements = reinterpret_cast<pci_requirements_t*>(&pci_requirements),
        .requirements_count = 4,
        .is_loaded = false,
};