//
// Created by Piotr on 13.05.2024.
//

#include "vmware-svgaii.hpp"

extern driver_type driver_entry_t vmware_svgaii_driver_descriptor;
static uint64_t vmw_io_port = 0;

static void vmware_write_reg(uint32_t reg, uint32_t value)
{
    outl(vmw_io_port + SVGA_INDEX, reg);
    outl(vmw_io_port + SVGA_VALUE, value);
}

static uint32_t vmware_read_reg(uint32_t reg)
{
    outl(vmw_io_port + SVGA_INDEX, reg);
    return inl(vmw_io_port + SVGA_VALUE);
}

static driver_handle_t vmware_svgaii_entry(pci_dev* pci_handle)
{
    kstd::printf("[VMW] Loaded VMWare SVGA II driver!\n");

    if (pci_handle->vendor_id != 0x15ad ||
        pci_handle->device_id != 0x0405)
    {
        kstd::printf("[VMW] Vendor ID or Device ID is invalid!\n");

        return {};
    }

    if (pci_handle == nullptr)
    {
        kstd::printf("PCI handle is null.\n");

        unreachable();
    }

    if (pci_handle->is_pcie)
    {
        pcie_enable_bus_mastering(pci_handle);
        pcie_enable_io_mastering(pci_handle);
        pcie_enable_mem_mastering(pci_handle);
    }

    pci_dev_header* hdr = reinterpret_cast<pci_dev_header*>(pci_handle->raw_pci_device_data);
    vmw_io_port = hdr->bars[0] - 1;

    kstd::printf("IO port: %lx\n", vmw_io_port);

    kstd::printf("%x\n", vmware_read_reg(SVGA_REG_FB_START));

    uint32_t specid = 0x90000002;

    vmware_write_reg(SVGA_REG_ID, specid);
    uint32_t v = vmware_read_reg(SVGA_REG_ID);
    if (v != specid)
    {
        kstd::printf("[VMW] Unsupported spec: %x\n", specid);
        //unreachable();
        return {};
    }
    vmware_write_reg(SVGA_REG_ENABLE, 1);

    vmware_svgaii_driver_descriptor.is_loaded = true;

    return {};
}

static driver_status_t vmw_set_res(
        [[maybe_unused]] driver_handle_t* vmw_handle,
        const GpuResolution* vmw_res
)
{
    if (vmware_svgaii_driver_descriptor.is_loaded) return DS_SUCCESS;
    auto vmw_w = vmw_res->width;
    auto vmw_h = vmw_res->height;
    auto vmw_bpp = vmw_res->bpp;

    kstd::printf("[VMW] Changing screen resolution to %ldx%ld @ %ld...\n", vmw_w, vmw_h, vmw_bpp);

    bool found = false;
    auto* lfb = Framebuffer::GetFramebuffer(0);
    limine_video_mode* tm;
    for (size_t i = 0; lfb->mode_count > i; i++)
    {
        auto* vm = lfb->modes[i];
        if (vm->height == vmw_h && vm->width == vmw_w && vm->bpp == vmw_bpp)
        {
            found = true;
            tm = vm;
            break;
        }
    }

    if (!found)
    {
        kstd::printf("No res mode found.");
        //unreachable();
        return DS_NO_GPU_MODE;
    }

    if (vmw_w > vmware_read_reg(SVGA_REG_MAX_WIDTH))
    {
        kstd::printf("[VMW] Unsupported width. Max width: %x\n", vmware_read_reg(SVGA_REG_MAX_WIDTH));

        return DS_NO_GPU_MODE;
    }

    if (vmw_h > vmware_read_reg(SVGA_REG_MAX_HEIGHT))
    {
        kstd::printf("[VMW] Unsupported width. Max height: %x\n", vmware_read_reg(SVGA_REG_MAX_HEIGHT));

        return DS_NO_GPU_MODE;
    }
    vmware_write_reg(SVGA_REG_ENABLE, 0);

    vmware_write_reg(SVGA_REG_WIDTH, vmw_w);
    auto vmw_w2 = vmware_read_reg(SVGA_REG_WIDTH);

    vmware_write_reg(SVGA_REG_HEIGHT, vmw_h);
    auto vmw_h2 = vmware_read_reg(SVGA_REG_HEIGHT);

    vmware_write_reg(SVGA_REG_BPP, vmw_bpp);
    auto vmw_bpp2 = vmware_read_reg(SVGA_REG_BPP);

    vmware_write_reg(SVGA_REG_ENABLE, 1);

    bool success = true;
    if (vmw_w2 != vmw_w)
    {
        kstd::printf("Failed to set the width.\n");
        success = false;
    }

    if (vmw_h2 != vmw_h)
    {
        kstd::printf("Failed to set the height.\n");
        success = false;
    }

    if (vmw_bpp2 != vmw_bpp)
    {
        kstd::printf("Failed to set the bpp.\n");
        success = false;
    }

    if (!success)
    {
        kstd::printf("Failure.\n");
        return DS_FAILURE;
    }

    // Update framebuffer.
    lfb->width = vmw_w;
    lfb->height = vmw_h;
    lfb->bpp = vmw_bpp;
    lfb->red_mask_shift = tm->red_mask_shift;
    lfb->red_mask_size = tm->red_mask_size;
    lfb->green_mask_shift = tm->green_mask_shift;
    lfb->green_mask_size = tm->green_mask_size;
    lfb->blue_mask_shift = tm->blue_mask_shift;
    lfb->blue_mask_size = tm->blue_mask_size;
    lfb->pitch = tm->pitch;
    lfb->memory_model = tm->memory_model;

    uint64_t base_address = vmware_read_reg(SVGA_REG_FB_START);
    base_address += vmware_read_reg(SVGA_REG_FB_OFFSET);
    base_address += vmm_hhdm->offset;

    lfb->address = reinterpret_cast<void*>(base_address);


    // Tell terminal to do the thing.
    kstd::reinit_term();

    return DS_SUCCESS;
}

static driver_status_t vmware_svgaii_ioctl(
     driver_handle_t* driver_handle,
     uint64_t driver_question,
     const char* question_buffer,
     [[maybe_unused]] char* ioctl_answer
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

static driver_status_t vmware_svgaii_cleanup()
{
    return DS_SUCCESS;
}

__attribute__((aligned(0x10))) static pci_requirements_t pci_requirements[] = {
    {
        .vendor_id = 0x15ad,
        .device_id = 0x0405,
        ._class = 0,
        .subclass = 0,
        .prog_if = 0,
        .match_requirement = PCI_REQ_VD,
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