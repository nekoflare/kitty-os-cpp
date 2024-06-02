//
// Created by Piotr on 03.05.2024.
//

#include "idt.hpp"
#include <kstd/kstdio.hpp>
#include <control/control.hpp>
#include "../../kernel_calls/kcalls.hpp"

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

static inline uint64_t read_cr2()
{
    uint64_t cr2;
    asm volatile ("mov %%cr2, %0" : "=r" (cr2));
    return cr2;
}

static void print_page_fault_info(Registers_x86_64& regs) {
    // Print the faulting address from CR2
    uint64_t faulting_address = read_cr2();
    kstd::printf("Linear address where the issue happened: %lx\n", faulting_address);

    // Decode the error code
    uint32_t error_code = regs.error_code;

    kstd::printf("The error has been caused by: ");

    if (error_code & (1 << 0)) {
        kstd::printf("page-protection violation. ");
    } else {
        kstd::printf("non-present page. ");
    }

    if (error_code & (1 << 1)) {
        kstd::printf("Write access. ");
    } else {
        kstd::printf("Read access. ");
    }

    if (error_code & (1 << 2)) {
        kstd::printf("User mode. ");
    } else {
        kstd::printf("Kernel mode. ");
    }

    if (error_code & (1 << 3)) {
        kstd::printf("Reserved bit violation. ");
    }

    if (error_code & (1 << 4)) {
        kstd::printf("Instruction fetch. ");
    }

    if (error_code & (1 << 5)) {
        kstd::printf("Protection key violation. ");
    }

    if (error_code & (1 << 6)) {
        kstd::printf("Shadow stack access violation. ");
    }

    if (error_code & (1 << 15)) {
        kstd::printf("SGX violation. ");
    }

    kstd::putc('\n');
    vmm_test(reinterpret_cast<void*>(faulting_address));

    kstd::printf("\n");
}

extern "C" void interrupt_handler(Registers_x86_64* regs)
{
    if (regs->interrupt_number == 0x81)
    {
        kcall_handler(regs);

        return;
    }

    kstd::printf("We've received an interrupt!\n");

    kstd::printf("RAX: %lx RBX: %lx RCX: %lx RDX: %lx\nR8: %lx R9: %lx R10: %lx R11: %lx\nR12: %lx R13: %lx R14: %lx R15: %lx\nRSI: %lx RDI: %lx RBP: %lx\nRSP: %lx CS@RIP: %lx@%lx\n",
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

        switch (regs->interrupt_number)
        {
            case 14:
                print_page_fault_info(*regs);

                break;
        }

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