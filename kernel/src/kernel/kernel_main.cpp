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

    //driver_ctrl_enumerate_drivers();

    vmm_map<pml4e*, uint64_t, uint64_t>(vmm_make_virtual<pml4e*, uint64_t>(vmm_get_pml4()), reinterpret_cast<uint64_t>(&kernel_main), 0x0, PROT_NONE, MAP_NONE, MISC_NONE);

    //pci_init();

    [[nodiscard]] while (true)
    {
        asm volatile ("nop");
    }
}