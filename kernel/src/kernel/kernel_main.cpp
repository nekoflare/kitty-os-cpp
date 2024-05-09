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

    mm_initialize();
    mm_enumerate_memmap_entries(true);

    mm_test();

    pml4e* _Pml4e = reinterpret_cast<pml4e*>(get_logical_address_pml4());
    size_t free_pml4e = 0;

    for (size_t i = 0; 256 > i; i++)
    {
        if (_Pml4e[i + 256].pdpe_base_address == 0 )
        {
            kstd::printf("E%ld of PML4E is free to take!\n", i + 256);
            free_pml4e = i + 256;
            break;
        }
    }

    // mm_map_pages(_Pml4e, 0xffffffff80000000, 0x6e000, 512, PROT_FORCE_MAP | PROT_RW | PROT_EXEC | PROT_SUPERVISOR, MAP_PRESENT, MISC_INVLPG, 0);

    //mm_map_page(_Pml4e, mm_create_va(false, free_pml4e, 0, 0, 0, 0), 0x6e000, PROT_RW | PROT_EXEC | PROT_SUPERVISOR, MAP_PRESENT, MISC_INVLPG, 0);

    pci_init();

    while (true)
    {
        asm volatile ("nop");
    }
}