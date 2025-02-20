//
// Created by Neko on 20.01.2025.
//

#include "idt.h"

#include <acpi/acpi.h>
#include <cstdint>
#include <dbg/log.h>
#include <hal/apic/apic.h>
#include <hal/irq/irq.h>
#include <mem/virtual.h>

#include "smp/smp.h"

const char *exception_strings[32] = {"(#DE) Division Error",
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
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "",
                                     "(#HV) Hypervisor Injection Exception",
                                     "(#VC) VMM Communication Exception",
                                     "(#SX) Security Exception",
                                     "(#--) Reserved"};

static inline uint64_t read_cr2()
{
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

static void print_page_fault_info(const interrupt_frame &regs)
{
    // Print the faulting address from CR2
    uint64_t faulting_address = read_cr2();
    debug_print("Linear address where the issue happened: %016lX\n", faulting_address);

    // Decode the error code
    uint32_t error_code = regs.error_code;

    debug_print("The error has been caused by: ");

    if (error_code & (1 << 0))
    {
        debug_print("page-protection violation. ");
    }
    else
    {
        debug_print("non-present page. ");
    }

    if (error_code & (1 << 1))
    {
        debug_print("Write access. ");
    }
    else
    {
        debug_print("Read access. ");
    }

    if (error_code & (1 << 2))
    {
        debug_print("User mode. ");
    }
    else
    {
        debug_print("Kernel mode. ");
    }

    if (error_code & (1 << 3))
    {
        debug_print("Reserved bit violation. ");
    }

    if (error_code & (1 << 4))
    {
        debug_print("Instruction fetch. ");
    }

    if (error_code & (1 << 5))
    {
        debug_print("Protection key violation. ");
    }

    if (error_code & (1 << 6))
    {
        debug_print("Shadow stack access violation. ");
    }

    if (error_code & (1 << 15))
    {
        debug_print("SGX violation. ");
    }

    debug_print("\n");
}

static void print_processor_flags(const uint64_t rflags)
{
    if (rflags & 0x00000001)
        debug_print("CF ");
    if (rflags & 0x00000004)
        debug_print("PF ");
    if (rflags & 0x00000010)
        debug_print("AF ");
    if (rflags & 0x00000040)
        debug_print("ZF ");
    if (rflags & 0x00000080)
        debug_print("SF ");
    if (rflags & 0x00000100)
        debug_print("TF ");
    if (rflags & 0x00000200)
        debug_print("IF ");
    if (rflags & 0x00000400)
        debug_print("DF ");
    if (rflags & 0x00000800)
        debug_print("OF ");
    if (rflags & 0x00010000)
        debug_print("RF ");
    if (rflags & 0x00020000)
        debug_print("VM ");
    if (rflags & 0x00040000)
        debug_print("AC ");
    if (rflags & 0x00080000)
        debug_print("VIF ");
    if (rflags & 0x00100000)
        debug_print("VIP ");
    if (rflags & 0x00200000)
        debug_print("ID ");
    if (rflags & 0x80000000)
        debug_print("AI ");
    debug_print("\n");
}

void print_registers(const interrupt_frame *regs)
{
    debug_print("RAX: %016lX RBX: %016lX RCX: %016lX RDX: %016lX\n", regs->rax, regs->rbx, regs->rcx, regs->rdx);
    debug_print("RSI: %016lX RDI: %016lX RBP: %016lX RSP: %016lX\n", regs->rsi, regs->rdi, regs->rbp, regs->rsp);
    debug_print("R8:  %016lX R9:  %016lX R10: %016lX R11: %016lX\n", regs->r8, regs->r9, regs->r10, regs->r11);
    debug_print("R12: %016lX R13: %016lX R14: %016lX R15: %016lX\n", regs->r12, regs->r13, regs->r14, regs->r15);
    debug_print("DR0: %016lX DR1: %016lX DR2: %016lX DR3: %016lX\n", regs->dr0, regs->dr1, regs->dr2, regs->dr3);
    debug_print("DR4: %016lX DR5: %016lX DR6: %016lX DR7: %016lX\n", regs->dr4, regs->dr5, regs->dr6, regs->dr7);
    debug_print("CR0: %016lX CR2: %016lX CR3: %016lX CR4: %016lX CR8: %016lX\n", regs->cr0, regs->cr2, regs->cr3,
                regs->cr4, regs->cr8);
    debug_print("CS:  %02lX  DS:  %02lX  SS:  %02lX  ES:  %02lX  FS:  %02lX GS: %02lX\n", regs->cs, regs->ds, regs->ss,
                regs->es, regs->fs, regs->gs);
    debug_print("RIP: %016lX\n", regs->rip);
    debug_print("Orig RSP: %016lX CR3: %016lX\n", regs->orig_rsp, regs->cr3);
    debug_print("Error code: %016lX Interrupt index: %016lX\n", regs->error_code, regs->interrupt_number);
    debug_print("RFLAGS: ");
    print_processor_flags(regs->rflags);
    // Print Debug Registers

    // Print Control Registers
    debug_print("%s\n", exception_strings[regs->interrupt_number]);
}

extern "C" void interrupt_handler(interrupt_frame *frame)
{
    if (frame->interrupt_number < 32)
    {
        print_registers(frame);

        if (frame->interrupt_number == 0xe)
        {
            print_page_fault_info(*frame);
        }
        asm volatile("cli; hlt");
        asm volatile("cli; hlt");
    }

    // tell the apic to chill tf out
    dispatch_irq(frame->interrupt_number);
    apic_send_eoi(get_lapic_address() + get_higher_half_memory_offset());
}

void initialize_idt()
{
    idtr idt_register = {.limit = sizeof(idt) - 1, .idt_address = reinterpret_cast<uint64_t>(&idt)};

    load_idt_table(&idt_register);

    asm volatile("sti");
}
