//
// Created by Piotr on 02.05.2024.
//

#include "kernel/fb/fb.hpp"
#include "kernel/hal/x64/gdt/gdt.hpp"
#include "control/control.hpp"
#include "kstd/kstdio.hpp"
#include <kernel/hal/x64/idt/idt.hpp>

extern "C" void kernel_main() {
    Framebuffer::Initialize();
    kstd::InitializeTerminal();
    flush_gdt();
    flush_idt();

    enable_interrupts();

    asm volatile ("int $0x80");

    kstd::printf("Dzien pierwszy:\n"
                 "Napisane dodatki do kernela: \n"
                 "1. GDT\n"
                 "2. Manipulacja pamięcia\n"
                 "3. Troche biblioteki matematyki\n"
                 "4. Grafika!\n"
                 "5. Wsparcie dla SSE i AVX.\n");

    kstd::printf("%f", (double)1.24);


    while (true) {
        unreachable();
    }
}