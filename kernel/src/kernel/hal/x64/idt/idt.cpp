//
// Created by Piotr on 03.05.2024.
//

#include "idt.hpp"

extern "C" void interrupt_handler(Registers_x86_64 regs)
{
    return;
}

void flush_idt()
{
    IDTR idtr = {
            .limit = sizeof(idt) - 1,
            .idt_address = reinterpret_cast<kstd::uint64_t>(&idt)
    };

    flush_idt_asm(&idtr);
}