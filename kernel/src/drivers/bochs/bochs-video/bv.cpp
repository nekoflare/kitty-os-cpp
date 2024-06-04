//
// Created by Piotr on 27.05.2024.
//

#include "bv.hpp"

extern driver_type driver_entry_t bv_drv_desc;

static driver_status_t bv_entry(pci_dev* pci_handle)
{
    kstd::printf("[BV] Starting...\n");

    bv_drv_desc.is_loaded = true;

    if (pci_handle->is_pcie)
    {
        kstd::printf("[BV] This device is using PCI-e.\n");
    }

    kstd::printf("[BV] Done!\n");
    return DS_SUCCESS;
}

static driver_status_t bv_cleanup()
{
    return DS_SUCCESS;
}

static driver_status_t bv_set_res(
        driver_handle_t* bv_handle,
        const GpuResolution* bv_res
)
{
    bool found = false;
    auto bv_w = bv_res->width;
    auto bv_h = bv_res->height;
    auto bv_bpp = bv_res->bpp;
    kstd::printf("[BV] Changing screen resolution to %ldx%ld @ %ld...\n", bv_res->width, bv_res->height, bv_res->bpp);

    auto* lfb = Framebuffer::GetFramebuffer(0);
    limine_video_mode* tm = nullptr;
    for (size_t i = 0; lfb->mode_count > i; i++)
    {
        auto* vm = lfb->modes[i];

        if (vm->height == bv_h && vm->width == bv_w && vm->bpp == bv_bpp)
        {
            found = true;
            tm = vm;
            break;
        }
    }

    if (!found)
    {
        return DS_NO_GPU_MODE;
    }

    // Disable VBE Extensions.
    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_ENABLE);
    outw(VBE_DISPI_IOPORT_DATA, 0);
    // VBE has been disabled.

    // Set Width and height
    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_XRES);
    outw(VBE_DISPI_IOPORT_DATA, bv_w);

    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_YRES);
    outw(VBE_DISPI_IOPORT_DATA, bv_h);

    // Set bpp
    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_BPP);
    outw(VBE_DISPI_IOPORT_DATA, bv_bpp);

    // Re-enable VBE extensions.
    outw(VBE_DISPI_IOPORT_INDEX, VBE_DISPI_INDEX_ENABLE);
    outw(VBE_DISPI_IOPORT_DATA, 1);


    // Update framebuffer.
    lfb->width = bv_w;
    lfb->height = bv_h;
    lfb->bpp = bv_bpp;
    lfb->red_mask_shift = tm->red_mask_shift;
    lfb->red_mask_size = tm->red_mask_size;
    lfb->green_mask_shift = tm->green_mask_shift;
    lfb->green_mask_size = tm->green_mask_size;
    lfb->blue_mask_shift = tm->blue_mask_shift;
    lfb->blue_mask_size = tm->blue_mask_size;
    lfb->pitch = tm->pitch;
    lfb->memory_model = tm->memory_model;

    // Tell terminal to do the thing.
    kstd::reinit_term();

    return DS_SUCCESS;
}

static driver_status_t bv_ioctl(
        driver_handle_t* bv_handle,
        uint64_t bv_ioctl,
        const char* bv_ioctl_buf,
        char * bv_ioctl_resp
)
{
    kstd::printf("[BV] IOCTL message: %lx\n", bv_ioctl);

    switch (bv_ioctl)
    {
        case GPU_SET_RESOLUTION:
            return bv_set_res(bv_handle, reinterpret_cast<const GpuResolution*>(bv_ioctl_buf));
        default:
            return DS_FAILURE;
    }

    return DS_FAILURE;
}

__attribute__((aligned(0x10))) static pci_requirements_t bv_pci_req[] = {
        {
            .vendor_id = 0x1234,
            .device_id = 0x1111,
            .match_requirement = PCI_REQ_VD
        }
};

driver_type driver_entry_t bv_drv_desc = {
        .driver_name = "Bochs video adapter driver.",
        .driver_author = "Piotr",
        .driver_description = "Simple driver to do simple things.",
        .driver_version = "Pre-release 1.00",
        .driver_designation = DT_GPU,
        .driver_load = DL_LOAD_ON_PCI_MATCH,
        .driver_entry = &bv_entry,
        .driver_cleanup = &bv_cleanup,
        .driver_ioctl = &bv_ioctl,
        .requirements = reinterpret_cast<pci_requirements_t*>(&bv_pci_req),
        .requirements_count = 1,
        .is_loaded = false
};