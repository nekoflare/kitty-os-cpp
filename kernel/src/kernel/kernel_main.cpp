//
// Created by Piotr on 02.05.2024.
//

#include <limine.h>
#include <firmware/acpi/acpi.hpp>
#include <public/kdu/driver_ctrl.hpp>
#include <exec/elf/loader.hpp>
#include <kterm/kt.hpp>
#include <kernel/debugging/debug_print.hpp>
#include <hal/bus/pci.hpp>
#include <sched/processes.hpp>
#include <hal/x64/tss/tss.hpp>
#include <kernel/clock.hpp>
#include <mm/heap.hpp>
#include <drivers/video/fb/fb.hpp>
#include <hal/x64/gdt/gdt.hpp>
#include <hal/x64/idt/idt.hpp>
#include <firmware/smbios/smbios.hpp>
#include <kernel/syscalls/syscalls.hpp>

extern void (*__init_array[])();
extern void (*__init_array_end[])();

limine_module_response* module_response = nullptr;

volatile limine_module_request module_request = {
        .id = LIMINE_MODULE_REQUEST,
        .revision = 0,
        .response = nullptr,
        .internal_module_count = 0,
        .internal_modules = nullptr,
};

void initialize_pci()
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

extern "C" void fuckme();

extern "C" void kernel_main()
{
    dbg_init();
    Framebuffer::Initialize();
    kstd::InitializeTerminal();

    vmm_init();
    pmm_init();
    heap_init();

    for (size_t i = 0; &__init_array[i] != __init_array_end; i++)
    {
        __init_array[i]();
    }

    flush_gdt();
    flush_idt();
    uniirq_init();
    enable_interrupts();

    clk_init();
    smbios_init();
    acpi_init();
    initialize_pci();

    driver_ctrl_call_ald();
    driver_ctrl_enumerate_drivers();
    smbios_dump_info();
    pmm_print_memory_information();

    sched_init();
    idt_enable_sched();
    tss_flush();

    sctbl_print_entries();

    kt_main();

    while (true)
    {
        asm volatile ("nop");
    }
}