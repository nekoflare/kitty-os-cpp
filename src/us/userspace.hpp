#pragma once

#include <cstdint>
#include <cstddef>

struct process_registers
{
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, r8, r9, r10, r11, r12, r13, r14, r15, rip, rflags;
    alignas(16) uint8_t sse_state[512]; // Space for SSE/AVX state
};

struct process
{
    const char* process_name;
    const char* process_path;
    uint64_t priority;
    uint64_t process_id;
    uint64_t parent_process_id; // Parent process ID
    uint64_t creation_time; // Process creation timestamp
    void* page_map;
    process_registers registers;
    void* stack_base;  // Stack base address
    size_t stack_size; // Stack size
    void* heap_base;   // Heap base address
    size_t heap_size;  // Heap size
    bool is_handled;   // Is this being currently handled by a CPU?
    process_state state; // Current state of the process
    int exit_code; // Exit code on termination
};


void* create_user_page_map();