#include <debug.hpp>
#include <hal/msr.hpp>
#include <us/syscalls.hpp>

void initialize_syscall(uint64_t syscall_handler, uint32_t cs_selector, uint32_t ss_selector, uint32_t user_cs_selector,
                        uint32_t user_ss_selector, uint64_t rflags_mask)
{
    uint64_t efer = rdmsr(MSR_IA32_EFER);
    efer |= EFER_SCE;
    wrmsr(MSR_IA32_EFER, efer);

    wrmsr(MSR_IA32_LSTAR, syscall_handler);

    uint64_t star = ((uint64_t)user_cs_selector << 48) |
                    ((uint64_t)user_ss_selector << 32) |
                    ((uint64_t)cs_selector << 16) |
                    ((uint64_t)ss_selector);
    wrmsr(MSR_IA32_STAR, star);

    wrmsr(MSR_IA32_FMASK, rflags_mask);
}

extern "C" void syscall_proper_handler(system_call_frame *frame)
{
    debug_printf("kocham piwo.\n");

    debug_printf("Register Values:\n");

    debug_printf("R15: 0x%016lX  R14: 0x%016lX  R13: 0x%016lX  R12: 0x%016lX\n", frame->r15, frame->r14, frame->r13,
                 frame->r12);
    debug_printf("RFLAGS: 0x%016lX  R10: 0x%016lX  R9: 0x%016lX  R8: 0x%016lX\n", frame->rflags, frame->r10, frame->r9,
                 frame->r8);
    debug_printf("RDI: 0x%016lX  RSI: 0x%016lX  RBP: 0x%016lX  RSP: 0x%016lX\n", frame->rdi, frame->rsi, frame->rbp,
                 frame->rsp);
    debug_printf("RDX: 0x%016lX  NEXT_INS: 0x%016lX  RBX: 0x%016lX  RAX: 0x%016lX\n", frame->rdx, frame->next_isns,
                 frame->rbx, frame->rax);
}
