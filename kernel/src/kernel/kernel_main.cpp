//
// Created by Piotr on 02.05.2024.
//

#include "kernel/fb/fb.hpp"
#include "kernel/hal/x64/gdt/gdt.hpp"
#include "control/control.hpp"
#include "kstd/kstdio.hpp"
// #include <kernel/hal/x64/idt/idt.hpp>
#include <kernel/drivers/sb16/sb16.hpp>

extern "C" void kernel_main() {
    Framebuffer::Initialize();
    kstd::InitializeTerminal();
    flush_gdt();
    //flush_idt();

    if (is_sb16_available())
    {
        kstd::printf("SB16 is available on the system.");
    }
    else
    {
        kstd::printf("No SB16 is on the system.");
    }

    while (true)
    {
        asm volatile ("nop");
    }
}