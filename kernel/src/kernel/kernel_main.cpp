//
// Created by Piotr on 02.05.2024.
//

#include "kernel/fb/fb.hpp"
#include "kernel/hal/x64/gdt/gdt.hpp"
#include "control/control.hpp"
#include "kstd/kstdio.hpp"
#include <kernel/hal/x64/idt/idt.hpp>
#include <kernel/memory/mm.hpp>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kernel/hal/x64/bus/pci/pci.hpp>
#include <kdu/driver_entry.hpp>
#include <kdu/driver_ctrl.hpp>
#include <kernel/memory/heap/heap.hpp>

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

    pmsa_initialize();
    //mm_enumerate_memmap_entries(true);

    // mm_test();

    //driver_ctrl_enumerate_drivers();

    //pci_init();

    heap_init();

    while (true)
        commit_page();

    [[nodiscard]] while (true)
    {
        asm volatile ("nop");
    }
}