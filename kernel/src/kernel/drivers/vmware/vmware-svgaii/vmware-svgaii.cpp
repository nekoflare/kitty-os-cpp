//
// Created by Piotr on 13.05.2024.
//

#include "vmware-svgaii.hpp"

extern driver_type driver_entry_t vmware_svgaii_driver_descriptor;
static uint32_t vmw_io_port = 0;

static driver_status_t vmware_svgaii_entry(pci_handle_t* pci_handle)
{
    kstd::printf("[VMW] Loaded VMWare SVGA II driver!\n");

    if (pci_handle->vendor_id != 0x15ad ||
        pci_handle->device_id != 0x0405)
    {
        kstd::printf("[VMW] Vendor ID or Device ID is invalid!\n");

        return DS_FAILURE;
    }

    vmw_io_port = reinterpret_cast<pci_device_header*>(pci_handle->full_header)->bar0 - 1;

    kstd::printf("[VMW] VMW IO Port: %lx\n", vmw_io_port);

    // Enable memory, io and bus mastering for it.

    vmware_svgaii_driver_descriptor.is_loaded = true;

    return DS_SUCCESS;
}

static driver_status_t vmware_svgaii_cleanup()
{
    kstd::printf("[VMW] Poof!\n");

    return DS_SUCCESS;
}

static driver_status_t vmw_set_res(
        driver_handle_t* vmw_handle,
        const GpuResolution* vmw_res
)
{
    bool found = false;
    auto vmw_w = vmw_res->width;
    auto vmw_h = vmw_res->height;
    auto vmw_bpp = vmw_res->bpp;

    kstd::printf("[VMW] Changing screen resolution to %lldx%lld @ %lld...\n", vmw_w, vmw_h, vmw_bpp);



    return DS_SUCCESS;
}

static driver_status_t vmware_svgaii_ioctl(
     driver_handle_t* driver_handle,
     uint64_t driver_question,
     const char* question_buffer,
     char* ioctl_answer
)
{
    switch (driver_question)
    {
        case GPU_SET_RESOLUTION:
            return vmw_set_res(driver_handle, reinterpret_cast<const GpuResolution*>(question_buffer));
        default:
            return DS_FAILURE;
    }


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