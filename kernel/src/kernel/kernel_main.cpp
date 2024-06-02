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
#include <kdu/driver_entry.hpp>
#include <kdu/driver_ctrl.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/memory/vmm.hpp>
#include <firmware/smbios/smbios.hpp>
#include <kernel/memory/pmm.hpp>
#include <firmware/acpi/acpi.hpp>
#include <kernel/hal/bus/pci.hpp>
#include <kdu/apis/graphics.hpp>
#include <kernel/memory/heap.hpp>
#include <kstd/kvector.hpp>
#include <exec/elf/loader.hpp>
#include <libs/cpuinfo/cpuinfo.hpp>

extern void (*__init_array[])();
extern void (*__init_array_end[])();

limine_module_response* module_response = nullptr;

volatile limine_module_request module_request = {
        .id = LIMINE_MODULE_REQUEST,
        .response = nullptr,
        .internal_module_count = 0,
        .internal_modules = nullptr
};

void aio_pci_init()
{
    if (acpi_get_mcfg() == nullptr)
    {
        kstd::printf("[Kernel] Using legacy PCI local bus.\n");
        pci_init();
    }
    else
    {
        kstd::printf("[Kernel] Using PCI express.\n");
        pcie_init();
    }
}

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
    heap_init();
    smbios_init();
    acpi_init();
    //irqs_init();
    //unreachable();
    //aio_pci_init();
    driver_ctrl_call_ald();
    driver_ctrl_enumerate_drivers();
    smbios_dump_info();
    pmm_print_memory_information();


    //pci_dev_hdr_common* hdr = pci_get_hdr(0, 0, 0);
    //pci_dev_enumerate(pci_dev_head);

    GpuResolution res = {
            .width = 1920,
            .height = 1080,
            .bpp = 32
    };
    driver_status_t st = ioctl_auto(DT_GPU, nullptr, GPU_SET_RESOLUTION, reinterpret_cast<const char*>(&res), nullptr);

    kstd::printf("ST = %llx\n", st);

    aio_pci_init();

    if (CPUInfo::IsAMD())
    {
        kstd::printf("This CPU is AMD.\n");
    }
    else
    {
        kstd::printf("This CPU is Intel.\n");
    }

    kstd::printf("Virtual bus width: %zu\n", CPUInfo::GetVirtualBusWidth());
    kstd::printf("Physical bus width: %zu\n", CPUInfo::GetPhysicalBusWidth());

    auto model_name = CPUInfo::GetCPUModelName();
    kstd::printf("Model name: %s\n", model_name);

    auto vendor_id = CPUInfo::GetCPUVendorID();
    kstd::printf("Vendor ID: %s\n", CPUInfo::GetCPUVendorID());

    if (CPUInfo::HasSSE())
        kstd::printf("SSE, ");
    if (CPUInfo::HasSSE2())
        kstd::printf("SSE2, ");
    if (CPUInfo::HasSSE3())
        kstd::printf("SSE3, ");
    if (CPUInfo::HasSupplementalSSE3())
        kstd::printf("SSSE3, ");
    if (CPUInfo::HasSSE4_1())
        kstd::printf("SSE4.1, ");
    if (CPUInfo::HasSSE4_2())
        kstd::printf("SSE4.2, ");

    [[nodiscard]] while (true)
    {
        asm volatile ("nop");
    }
}