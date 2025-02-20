//
// Created by Neko on 25.01.2025.
//

#ifndef SCHED_H
#define SCHED_H

#include <cstdint>
#include <string>
#include <vector>

#include "kernel.h"
#include "mem/virtual.h"

constexpr size_t stack_size = 1 * 1024 * 1024;

struct process_context;
struct process_thread;

enum process_level
{
    USER,
    KERNEL
};

struct vmem_entry
{
    uint64_t base;
    size_t length;
};

struct process_thread
{
    uint64_t thread_id;
    process_context *process;
    uint64_t cr0, cr3, cr4, cr8, r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rsp, rdx, rcx, rbx, rax, rip,
        rflags, orig_rsp, cs, ss, es, ds, fs, gs;
    uint64_t dr0, dr1, dr2, dr3, dr4, dr5, dr6, dr7;
};

struct process_context
{
    uint64_t process_id;
    std::string process_path;
    std::string process_name;
    uint8_t sse_state[256];
    bool is_paused;
    bool is_being_debugged;
    uint64_t debugger_pid;
    process_level level;
    std::vector<process_thread> threads;
    pml4 *page_map; // this has to contain 512 entries
    std::vector<vmem_entry> available_vmem;
};

handle create_process(std::string process_path, std::string process_name, void *entry_point, const process_level level,
                      const bool start_paused);

std::pair<bool, uint64_t> allocate_virtual_memory_at(process_context &ctx, uint64_t requested_base, size_t size);
std::pair<bool, uint64_t> allocate_virtual_memory(process_context &ctx, size_t size);

bool allocate_memory_in_process(handle process, uint64_t at_address, uint64_t page_count);

bool process_write(handle process, uint64_t at_address, void *buffer, size_t buffer_size);

bool process_read(handle process, uint64_t from_address, void *buffer, size_t buffer_size);

#endif // SCHED_H
