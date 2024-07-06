//
// Created by Piotr on 03.05.2024.
//

#include <hal/x64/idt/idt.hpp>
#include <kstd/kstdio.hpp>
#include <arch/x64/control/control.hpp>
#include <hal/x64/irqs/pic/pic.hpp>
#include <sched/processes.hpp>

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
static void print_rflags(uint64_t rflags)
{
    if (rflags & 0x00000001) kstd::printf("CF ");
    if (rflags & 0x00000004) kstd::printf("PF ");
    if (rflags & 0x00000010) kstd::printf("AF ");
    if (rflags & 0x00000040) kstd::printf("ZF ");
    if (rflags & 0x00000080) kstd::printf("SF ");
    if (rflags & 0x00000100) kstd::printf("TF ");
    if (rflags & 0x00000200) kstd::printf("IF ");
    if (rflags & 0x00000400) kstd::printf("DF ");
    if (rflags & 0x00000800) kstd::printf("OF ");
    if (rflags & 0x00010000) kstd::printf("RF ");
    if (rflags & 0x00020000) kstd::printf("VM ");
    if (rflags & 0x00040000) kstd::printf("AC ");
    if (rflags & 0x00080000) kstd::printf("VIF ");
    if (rflags & 0x00100000) kstd::printf("VIP ");
    if (rflags & 0x00200000) kstd::printf("ID ");
    if (rflags & 0x80000000) kstd::printf("AI ");
    kstd::printf("\n");
}

void print_registers(Registers_x86_64* regs)
{
    kstd::enable_tailing_zeroes();
    kstd::printf("RAX: %lx RBX: %lx RCX: %lx RDX: %lx\n", regs->rax, regs->rbx, regs->rcx, regs->rdx);
    kstd::printf("RSI: %lx RDI: %lx RBP: %lx RSP: %lx\n", regs->rsi, regs->rdi, regs->rbp, regs->rsp);
    kstd::printf("R8:  %lx R9:  %lx R10: %lx R11: %lx\n", regs->r8,  regs->r9,  regs->r10, regs->r11);
    kstd::printf("R12: %lx R13: %lx R14: %lx R15: %lx\n", regs->r12, regs->r13, regs->r14, regs->r15);
    kstd::printf("RIP: %lx\n", regs->rip);
    kstd::disable_tailing_zeroes();
    kstd::printf("CS: %lx  DS: %lx SS: %lx ES: %lx FS: %lx GS: %lx\n", regs->cs, regs->ds, regs->ss, regs->es, regs->fs, regs->gs);
    kstd::enable_tailing_zeroes();
    kstd::printf("Orig RSP: %lx CR3: %lx\n", regs->orig_rsp, regs->cr3);
    kstd::printf("Error code: %lx Interrupt index: %lx\n", regs->error_code, regs->interrupt_number);
    kstd::disable_tailing_zeroes();
    kstd::printf("RFLAGS: ");
    print_rflags(regs->rflags);
    kstd::printf("%s\n", exception_strings[regs->interrupt_number]);
}

bool enabled_sched = false;

extern "C" void interrupt_handler(Registers_x86_64* regs)
{
    if (regs->interrupt_number >= 0x90 && regs->interrupt_number <= 0xa0)
    {
        uint64_t irq = regs->interrupt_number - 0x90;

        idt_internal_call(regs->interrupt_number, regs);

        pic_send_eoi(irq);
    }

    if (regs->interrupt_number < 32)
    {
        kstd::enable_tailing_zeroes();
        kstd::printf("We've received an interrupt!\n");
        print_registers(regs);

        switch (regs->interrupt_number)
        {
            case 1:
                return;
            case 14:
                print_page_fault_info(*regs);

                break;
        }

        unreachable();
    }

    if (enabled_sched && regs->interrupt_number == 0x90)
    {
        proc_scheduler(regs);
    }

    return;
}

void idt_enable_sched()
{
    enabled_sched = true;
}

void flush_idt()
{
    IDTR idtr = {
            .limit = sizeof(idt) - 1,
            .idt_address = reinterpret_cast<uint64_t>(&idt)
    };

    flush_idt_asm(&idtr);
}

intr* head = nullptr;
void idt_attach_interrupt(int int_idx, idt_function_pointer fn) {
    intr* new_intr = new intr;
    new_intr->int_idx = int_idx;
    new_intr->fn = fn;
    new_intr->next = nullptr;

    if (head == nullptr) {
        head = new_intr;
        return;
    }

    intr* current = head;
    while (current->next != nullptr) {
        current = current->next;
    }
    current->next = new_intr;
}

void hook_interrupt(int int_idx, idt_function_pointer fn)
{
    idt_attach_interrupt(int_idx, fn);
}
void idt_internal_call(int int_idx, Registers_x86_64* regs)
{
    auto thead = head;
    while (thead != nullptr)
    {
        if (thead->int_idx == int_idx)
        {
            thead->fn(regs);
        }

        thead = thead->next;
    }
}