//
// Created by Piotr on 03.05.2024.
//

#include "idt.hpp"
#include <kstd/kstdio.hpp>
#include <control/control.hpp>

const char* exception_strings[32] = {
        "(#DE) Division Error",
        "(#DB) Debug",
        "(#--) Non-maskable Interrupt",
        "(#BP) Breakpoint",
        "(#OF) Overflow",
        "(#BR) Bound Range Exceeded",
        "(#UD) Invalid Opcode",
        "(#NM) Device Not Available",
        "(#DF) Double Fault",
        "(#--) Coprocessor Segment Overrun",
        "(#TS) Invalid TSS",
        "(#NP) Segment Not Present",
        "(#SS) Stack Segment Fault",
        "(#GP) General Protection Fault",
        "(#PF) Page Fault",
        "(#--) Reserved",
        "(#MF) x87 Floating-Point Exception",
        "(#AC) Alignment Check",
        "(#MC) Machine Check",
        "(#XM) SIMD Floating-Point Exception",
        "(#VE) Virtualization Exception",
        "(#CP) Control Protection Exception",
        "", "", "", "", "", "",
        "(#HV) Hypervisor Injection Exception",
        "(#VC) VMM Communication Exception",
        "(#SX) Security Exception",
        "(#--) Reserved"
};

void (*custom_interrupt_handlers[256])(Registers_x86_64*) = { nullptr };

bool hook_interrupt(size_t index, void (*new_interrupt_handler)(Registers_x86_64*))
{
    if (custom_interrupt_handlers[index] != nullptr)
    {
        custom_interrupt_handlers[index] = new_interrupt_handler;
        return true;
    }

    return false;
}

extern "C" void interrupt_handler(Registers_x86_64* regs)
{
    kstd::printf("We've received an interrupt!\n");

    kstd::printf("RAX: %llx RBX: %llx RCX: %llx RDX: %llx\nR8: %llx R9: %llx R10: %llx R11: %llx\nR12: %llx R13: %llx R14: %llx R15: %llx\nRSI: %llx RDI: %llx RBP: %llx\nRSP: %llx CS@RIP: %llx@%llx\n",
                 regs->rax, regs->rbx, regs->rcx, regs->rdx,
                 regs->r8, regs->r9, regs->r10, regs->r11,
                 regs->r12, regs->r13, regs->r13, regs->r15,
                 regs->rsi, regs->rdi, regs->rbp, regs->rsp,
                 regs->cs, regs->rip);

    if (custom_interrupt_handlers[regs->interrupt_number] != nullptr)
    {
        custom_interrupt_handlers[regs->interrupt_number](regs);

        return;
    }

    if (regs->interrupt_number < 32)
    {
        kstd::printf("%s\n", exception_strings[regs->interrupt_number]);

        unreachable();
    }

    return;
}


void flush_idt()
{
    IDTR idtr = {
            .limit = sizeof(idt) - 1,
            .idt_address = reinterpret_cast<uint64_t>(&idt)
    };

    flush_idt_asm(&idtr);
}