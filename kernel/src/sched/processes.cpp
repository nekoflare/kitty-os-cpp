#include <kstd/kmutex.hpp>
#include <mm/vmm.hpp>
#include <sched/processes.hpp>

uint64_t last_pid = 0;
process_t* proc_head = nullptr;
process_t* current_process = nullptr;

kstd::mutex proc_mtx;

void sched_init()
{
    proc_mtx.unlock();
}

kstd::mutex add_task_mtx;

process_t* proc_create_raw_process(const char* name, uint64_t prio)
{
    add_task_mtx.lock();

    auto proc = new process_t(proc_alloc_id(), name, {}, false, prio);

    proc->next = proc_head;
    proc_head = proc;

    add_task_mtx.unlock();

    return proc;
}

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

    kstd::printf("The RIP: %lx\n", reinterpret_cast<uint64_t>(task_pointer));
    kstd::printf("The RSP: %lx\n", proc.registers.rsp);

    proc_mtx.unlock();
}

void proc_print_all_processes()
{
    kstd::printf("Processes: \n");
    proc_mtx.lock();

    process_t* current = proc_head;
    while (current != nullptr)
    {
        kstd::printf("PID: %lx Name: %s\n", current->process_id, current->process_name);
        current = current->next;
    }

    proc_mtx.unlock();
}
bool dirty_fix = false;

void proc_scheduler(Registers_x86_64* regs)
{
    proc_mtx.lock();

    if (proc_head == nullptr)
    {
        proc_mtx.unlock();
        return;
    }

    if (current_process != nullptr)
    {
        // Save the current context (registers) of the running process
        current_process->registers.rip = regs->rip;
        current_process->registers.orig_rsp = regs->rsp;
        current_process->registers.rflags = regs->rflags;
        current_process->registers.cs = regs->cs;
        current_process->registers.ds = regs->ds;
        current_process->registers.ss = regs->ss;
        current_process->registers.es = regs->es;
        current_process->registers.fs = regs->fs;
        current_process->registers.gs = regs->gs;
        current_process->registers.rax = regs->rax;
        current_process->registers.rbx = regs->rbx;
        current_process->registers.rcx = regs->rcx;
        current_process->registers.rdx = regs->rdx;
        current_process->registers.rsi = regs->rsi;
        current_process->registers.rdi = regs->rdi;
        current_process->registers.rbp = regs->rbp;
        current_process->registers.r8 = regs->r8;
        current_process->registers.r9 = regs->r9;
        current_process->registers.r10 = regs->r10;
        current_process->registers.r11 = regs->r11;
        current_process->registers.r12 = regs->r12;
        current_process->registers.r13 = regs->r13;
        current_process->registers.r14 = regs->r14;
        current_process->registers.r15 = regs->r15;
        current_process->registers.cr3 = regs->cr3; // Save the CR3 register
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

    // Restore the context (registers) of the next process to run
    regs->rip = current_process->registers.rip;
    regs->orig_rsp = current_process->registers.rsp;
    regs->rflags = current_process->registers.rflags;
    regs->cs = current_process->registers.cs;
    regs->ds = current_process->registers.ds;
    regs->ss = current_process->registers.ss;
    regs->es = current_process->registers.es;
    regs->fs = current_process->registers.fs;
    regs->gs = current_process->registers.gs;
    regs->rax = current_process->registers.rax;
    regs->rbx = current_process->registers.rbx;
    regs->rcx = current_process->registers.rcx;
    regs->rdx = current_process->registers.rdx;
    regs->rsi = current_process->registers.rsi;
    regs->rdi = current_process->registers.rdi;
    regs->rbp = current_process->registers.rbp;
    regs->r8 = current_process->registers.r8;
    regs->r9 = current_process->registers.r9;
    regs->r10 = current_process->registers.r10;
    regs->r11 = current_process->registers.r11;
    regs->r12 = current_process->registers.r12;
    regs->r13 = current_process->registers.r13;
    regs->r14 = current_process->registers.r14;
    regs->r15 = current_process->registers.r15;
    regs->cr3 = current_process->registers.cr3; // Restore the CR3 register

    proc_mtx.unlock();
}
