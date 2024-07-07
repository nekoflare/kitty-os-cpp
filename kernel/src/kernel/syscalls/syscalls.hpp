#ifndef KKC_SYSCALLS_HPP
#define KKC_SYSCALLS_HPP

#include <cstdint>
#include <cstddef>
#include <hal/x64/idt/idt.hpp>

#define syscall_type __attribute__((section(".syscalls"), used))

struct syscall_entry;

// syscall_entry* self, Registers* registers
typedef void(*syscall_fn)(syscall_entry*, Registers_x86_64*);

struct syscall_entry
{
    size_t syscall_id;
    syscall_fn syscall_function;
};

void sctbl_print_entries();
void sctbl_call(Registers_x86_64* regs);

#endif // KKC_SYSCALLS_HPP
