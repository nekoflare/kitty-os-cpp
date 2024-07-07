#include <kstd/kstdio.hpp>
#include "syscalls.hpp"

extern char *_syscall_tbl_start;
extern char *_syscall_tbl_end;

void sctbl_print_entries()
{
    auto diff = (_syscall_tbl_end - _syscall_tbl_start) / sizeof(syscall_entry);

    kstd::printf("%zu\n", diff);
}

void sctbl_call(Registers_x86_64* regs)
{
    auto diff = (_syscall_tbl_end - _syscall_tbl_start) / sizeof(syscall_entry);
    syscall_entry* scent = reinterpret_cast<syscall_entry*>(_syscall_tbl_start);

    for (size_t i = 0; diff > i; i++)
    {
        auto ent = scent[i];
        if (ent.syscall_id == regs->rax)
        {
            ent.syscall_function(&scent[i], regs);
        }
    }
}