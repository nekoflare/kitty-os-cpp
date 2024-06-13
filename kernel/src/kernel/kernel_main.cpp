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
#include <kernel/irqs/uniirq.hpp>
// #include <devices/ps2/mouse/ps2_mouse.hpp>
#include <devices/ata/ata.hpp>
#include <ramvfs/ramvfs.hpp>
#include <libs/imgdraw/imgdraw.hpp>
#include <kernel/clock.hpp>
#include <debugging/debug_print.hpp>
#include <kernel/hal/x64/tss/tss.hpp>
#include <kernel/proc/processes.hpp>
#include <kernel/kbd.hpp>
#include <kernel_terminal/kt.hpp>
#include <devices/rtc/rtc.hpp>

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

void memset_qw(void *ptr, uint64_t value, size_t count) {
    uint64_t *p = (uint64_t *)ptr;
    for (size_t i = 0; i < count; ++i) {
        p[i] = value;
    }
}

extern "C" void test_func();

extern "C" void kernel_main()
{
    for (size_t i = 0; &__init_array[i] != __init_array_end; i++)
    {
        __init_array[i]();
    }

    dbg_init();
    Framebuffer::Initialize();
    kstd::InitializeTerminal();
    flush_gdt();
    flush_idt();
    uniirq_init();
    enable_interrupts();

    vmm_init();
    pmm_init();
    heap_init();
    clk_init();
    smbios_init();
    acpi_init();
    aio_pci_init();

    driver_ctrl_call_ald();
    //driver_ctrl_enumerate_drivers();
    //smbios_dump_info();
    //pmm_print_memory_information();

    GpuResolution res = {
            .width = 2048,
            .height = 1536,
            .bpp = 32
    };
    ioctl_auto(DT_GPU, nullptr, GPU_SET_RESOLUTION, reinterpret_cast<const char*>(&res), nullptr);

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
    if (CPUInfo::HasSSE4a())
        kstd::printf("SSE4a, ");
    if (CPUInfo::HasSSE4_1())
        kstd::printf("SSE4.1, ");
    if (CPUInfo::HasSSE4_2())
        kstd::printf("SSE4.2, ");
    if (CPUInfo::HasAVX())
        kstd::printf("AVX, ");
    if (CPUInfo::HasAVX2())
        kstd::printf("AVX2, ");
    if (CPUInfo::HasAVX512_F())
        kstd::printf("AVX512-F");
    kstd::printf("\n");

    // ps2_mouse_init();
    //pci_dump_database();

    tss_flush();
    sched_init();
    kstd::printf("Creating new task.\n");
    proc_create_task(10, "krnl.exe", &test_func);
    kstd::printf("Printing all processes.\n");
    proc_print_all_processes();
    kstd::printf("Enabling sched.\n");
    idt_enable_sched();
    

    kt_main();

    [[nodiscard]] while (true)
    {
        asm volatile ("nop");
    }
}