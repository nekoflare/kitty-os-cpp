//
// Created by Piotr on 07.06.2024.
//

#ifndef KITTY_OS_CPP_PROCESSES_HPP
#define KITTY_OS_CPP_PROCESSES_HPP

#include <kernel/hal/x64/idt/idt.hpp>
#include <kstd/kstring.hpp>

struct process_t
{
    uint64_t process_id;
    const char* process_name;
    Registers_x86_64 registers;
    bool is_being_processed;
    uint64_t priority;
    process_t* next; // Pointer to the next process in the list

    // Constructor for convenience
    process_t(uint64_t id, const char* name, const Registers_x86_64& regs, bool is_proc, uint64_t pri)
            : process_id(id), process_name(kstd::strdup(name)), registers(regs), is_being_processed(is_proc), priority(pri), next(nullptr) {}

    // Destructor to free allocated memory
    ~process_t() {
        delete[] process_name;
    }
};

void proc_add_task(const process_t& proc);
bool proc_remove_task(uint64_t process_id);
void proc_create_task(uint64_t prio, const char* name, void(*task_pointer)());
void proc_print_all_processes();
void proc_scheduler(Registers_x86_64* regs);
void sched_init();

#endif //KITTY_OS_CPP_PROCESSES_HPP
