//
// Created by Piotr on 02.05.2024.
//

#include "kernel/fb/fb.hpp"
#include "kernel/hal/x64/gdt/gdt.hpp"
#include "control/control.hpp"
#include "kstd/kstdio.hpp"
#include <kernel/hal/x64/idt/idt.hpp>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kernel/hal/x64/bus/pci/pci.hpp>
#include <kdu/driver_entry.hpp>
#include <kdu/driver_ctrl.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/memory/vmm.hpp>
#include <firmware/smbios/smbios.hpp>
#include <kernel/memory/pmm.hpp>
#include <firmware/acpi/acpi.hpp>
#include <kernel/hal/x64/bus/pci-e/pci-e.hpp>

#include <kdu/apis/graphics.hpp>

extern void (*__init_array[])();
extern void (*__init_array_end[])();

extern "C" void kernel_main()
{
    for (size_t i = 0; &__init_array[i] != __init_array_end; i++)
    {
        __init_array[i]();
    }

    Framebuffer::Initialize();
    kstd::InitializeTerminal();
    flush_gdt();
    flush_idt();
    enable_interrupts();

    vmm_init();
    pmm_init();
    pmm_print_memory_information();

    smbios_init();
    acpi_init();

    driver_ctrl_enumerate_drivers();

    pci_init();
    pcie_init();
    driver_ctrl_call_ald();

    auto* lfb = Framebuffer::GetFramebuffer(0);
    auto* vm = lfb->modes;
    auto vmc = lfb->mode_count;

    for (size_t i = 0; vmc > i; i++)
    {
        kstd::printf("%lldx%lld@%lld\n", vm[i]->width, vm[i]->height, vm[i]->bpp);
    }

    GpuResolution res = {
            .width = 1920,
            .height = 1080,
            .bpp = 32
    };
    driver_status_t st = ioctl_auto(DT_GPU, nullptr, GPU_SET_RESOLUTION, reinterpret_cast<const char*>(&res), nullptr);

    kstd::printf("ST = %llx\n", st);
    /*
    driver_ctrl_enumerate_drivers();
    smbios_dump_info();
    pmm_print_memory_information();
*/
    [[nodiscard]] while (true)
    {
        asm volatile ("nop");
    }
}