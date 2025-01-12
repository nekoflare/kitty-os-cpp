#pragma once

#include <stdint.h>

#define MSR_IA32_LSTAR 0xC0000082 // Target RIP for SYSCALL
#define MSR_IA32_STAR 0xC0000081  // Segment selectors
#define MSR_IA32_FMASK 0xC0000084 // Flags mask
#define MSR_IA32_EFER 0xC0000080  // Extended Feature Enable Register
#define EFER_SCE (1 << 0)         // SYSCALL/SYSRET enable bit

struct system_call_frame
{
    uint64_t r15, r14, r13, r12, rflags, r10, r9, r8, rdi, rsi, rbp, rsp, rdx, next_isns, rbx, rax;
} __attribute__((packed));

extern "C" void syscall_handler();
void initialize_syscall(uint64_t syscall_handler, uint32_t cs_selector, uint32_t ss_selector, uint32_t user_cs_selector,
                        uint32_t user_ss_selector, uint64_t rflags_mask);