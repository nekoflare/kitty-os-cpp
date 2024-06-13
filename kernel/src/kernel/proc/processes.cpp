#include <kstd/kmutex.hpp>
#include <kernel/memory/vmm.hpp>
#include "processes.hpp"

uint64_t last_pid = 0;
process_t* proc_head = nullptr;
process_t* current_process = nullptr;

kstd::mutex proc_mtx;

void sched_init()
{
    proc_mtx.unlock();
}

kstd::mutex add_task_mtx;
void proc_add_task(const process_t& proc)
{
    add_task_mtx.lock();

    // Dynamically allocate a new process_t instance
    process_t* new_proc = new process_t(proc.process_id, proc.process_name, proc.registers, proc.is_being_processed, proc.priority);

    // Add the new process to the beginning of the list
    new_proc->next = proc_head;
    proc_head = new_proc;

    add_task_mtx.unlock();
}

bool proc_remove_task(uint64_t process_id)
{
    proc_mtx.lock();

    process_t* current = proc_head;
    process_t* previous = nullptr;

    while (current != nullptr)
    {
        if (current->process_id == process_id)
        {
            if (previous != nullptr)
            {
                previous->next = current->next;
            }
            else
            {
                proc_head = current->next;
            }
            delete current;
            proc_mtx.unlock();
            return true;
        }
        previous = current;
        current = current->next;
    }
    proc_mtx.unlock();
    return false;
}

kstd::mutex pai_mtx;
uint64_t proc_alloc_id()
{
    pai_mtx.lock();
    last_pid++;
    pai_mtx.unlock();
    return last_pid;
}

void proc_create_task(uint64_t prio, const char* name, void(*task_pointer)())
{
    proc_mtx.lock();

    process_t proc(proc_alloc_id(), name, {}, false, prio);

    kstd::memset(&proc.registers, 0, sizeof(decltype(proc.registers)));

    proc.registers.rip = reinterpret_cast<uint64_t>(task_pointer);
    proc.registers.rsp = reinterpret_cast<uint64_t>(new uint8_t[4 * 4096]);
    proc.registers.rflags = 0x200; // Enable interrupts.
    proc.registers.cr3 = vmm_read_cr3();
    proc.registers.cs = 0x8;
    proc.registers.ds = 0x10;
    proc.registers.ss = 0x10;
    proc.registers.es = 0x10;
    proc.registers.fs = 0x10;
    proc.registers.gs = 0x10;
    proc.registers.rbp = proc.registers.rsp;

    proc_add_task(proc);

    kstd::printf("The RIP: %llx\n", reinterpret_cast<uint64_t>(task_pointer));
    kstd::printf("The RSP: %llx\n", proc.registers.rsp);

    proc_mtx.unlock();
}

void proc_print_all_processes()
{
    kstd::printf("Processes: \n");
    proc_mtx.lock();

    process_t* current = proc_head;
    while (current != nullptr)
    {
        kstd::printf("PID: %llx Name: %s\n", current->process_id, current->process_name);
        current = current->next;
    }

    proc_mtx.unlock();
}

void proc_scheduler(Registers_x86_64* regs)
{
    kstd::printf("Lock me up daddy.\n");

    proc_mtx.lock();

    kstd::printf("Scheding this shit.\n");

    print_registers(regs);

    // scheduling it is.
    if (proc_head == nullptr)
    {
        proc_mtx.unlock();
        return;
    }

    if (current_process == nullptr)
    {
        current_process = proc_head;
    }
    else
    {
        current_process = current_process->next;
        if (current_process == nullptr)
        {
            current_process = proc_head; // Loop back to the beginning of the list
        }
    }

    auto v = regs->orig_rsp;

    // do fuckery with registers.
    kstd::memcpy(regs, &current_process->registers, sizeof(Registers_x86_64));

    regs->orig_rsp = v;
    regs->ss = 0x10;
    regs->cs = 0x8;

    print_registers(&current_process->registers);

    kstd::printf("Scheduling...\n");
    bochs_breakpoint();

    proc_mtx.unlock();
}
