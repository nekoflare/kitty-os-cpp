//
// Created by Piotr on 03.05.2024.
//

#include <acpi/acpi.hpp>
#include <debug.hpp>
#include <hal/apic/apic.hpp>
#include <hal/hal.hpp>
#include <hal/idt/idt.hpp>
#include <mem/virtual_memory.hpp>
#include <sys/symbols.hpp>

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

static void print_page_fault_info(interrupt_frame &regs)
{
    // Print the faulting address from CR2
    uint64_t faulting_address = read_cr2();
    debug_printf("Linear address where the issue happened: %016lX\n", faulting_address);

    // Decode the error code
    uint32_t error_code = regs.error_code;

    debug_printf("The error has been caused by: ");

    if (error_code & (1 << 0))
    {
        debug_printf("page-protection violation. ");
    }
    else
    {
        debug_printf("non-present page. ");
    }

    if (error_code & (1 << 1))
    {
        debug_printf("Write access. ");
    }
    else
    {
        debug_printf("Read access. ");
    }

    if (error_code & (1 << 2))
    {
        debug_printf("User mode. ");
    }
    else
    {
        debug_printf("Kernel mode. ");
    }

    if (error_code & (1 << 3))
    {
        debug_printf("Reserved bit violation. ");
    }

    if (error_code & (1 << 4))
    {
        debug_printf("Instruction fetch. ");
    }

    if (error_code & (1 << 5))
    {
        debug_printf("Protection key violation. ");
    }

    if (error_code & (1 << 6))
    {
        debug_printf("Shadow stack access violation. ");
    }

    if (error_code & (1 << 15))
    {
        debug_printf("SGX violation. ");
    }

    print_page_info(faulting_address);

    debug_printf("\n");
}
static void print_processor_flags(uint64_t rflags)
{
    if (rflags & 0x00000001)
        debug_printf("CF ");
    if (rflags & 0x00000004)
        debug_printf("PF ");
    if (rflags & 0x00000010)
        debug_printf("AF ");
    if (rflags & 0x00000040)
        debug_printf("ZF ");
    if (rflags & 0x00000080)
        debug_printf("SF ");
    if (rflags & 0x00000100)
        debug_printf("TF ");
    if (rflags & 0x00000200)
        debug_printf("IF ");
    if (rflags & 0x00000400)
        debug_printf("DF ");
    if (rflags & 0x00000800)
        debug_printf("OF ");
    if (rflags & 0x00010000)
        debug_printf("RF ");
    if (rflags & 0x00020000)
        debug_printf("VM ");
    if (rflags & 0x00040000)
        debug_printf("AC ");
    if (rflags & 0x00080000)
        debug_printf("VIF ");
    if (rflags & 0x00100000)
        debug_printf("VIP ");
    if (rflags & 0x00200000)
        debug_printf("ID ");
    if (rflags & 0x80000000)
        debug_printf("AI ");
    debug_printf("\n");
}

void print_registers(interrupt_frame *regs)
{
    debug_printf("RAX: %016lX RBX: %016lX RCX: %016lX RDX: %016lX\n", regs->rax, regs->rbx, regs->rcx, regs->rdx);
    debug_printf("RSI: %016lX RDI: %016lX RBP: %016lX RSP: %016lX\n", regs->rsi, regs->rdi, regs->rbp, regs->rsp);
    debug_printf("R8:  %016lX R9:  %016lX R10: %016lX R11: %016lX\n", regs->r8, regs->r9, regs->r10, regs->r11);
    debug_printf("R12: %016lX R13: %016lX R14: %016lX R15: %016lX\n", regs->r12, regs->r13, regs->r14, regs->r15);
    debug_printf("RIP: %016lX\n", regs->rip);
    debug_printf("CS: %02lX  DS: %02lX SS: %02lX ES: %02lX FS: %02lX GS: %02lX\n", regs->cs, regs->ds, regs->ss,
                 regs->es, regs->fs, regs->gs);
    debug_printf("Orig RSP: %016lX CR3: %016lX\n", regs->orig_rsp, regs->cr3);
    debug_printf("Error code: %016lX Interrupt index: %016lX\n", regs->error_code, regs->interrupt_number);
    debug_printf("RFLAGS: ");
    print_processor_flags(regs->rflags);
    debug_printf("%s\n", exception_strings[regs->interrupt_number]);
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

        asm volatile("cli; hlt"); // exception.
    }

    auto lapic_id = get_lapic_id(); // aka cpu id
    receive_irq(frame->interrupt_number - 0x20, lapic_id);

    // anything else is just apic telling me theres an interrupt.
    apic_send_eoi(get_lapic_address());
}

void load_idt()
{
    idtr idt_register = {.limit = sizeof(idt) - 1, .idt_address = reinterpret_cast<uint64_t>(&idt)};

    flush_idt_asm(&idt_register);
    enable_interrupts();
}

EXPORT_SYMBOL(enable_interrupts);
EXPORT_SYMBOL(disable_interrupts);