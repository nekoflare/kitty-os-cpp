//
// Created by Neko on 25.01.2025.
//

#include "sched.h"
#include <utility>
#include <vector>
#include <cstring>
#include "dbg/log.h"
#include "kernel.h"
#include "mem/physical.h"

std::vector<process_context> processes;

handle create_process(std::string process_path, std::string process_name, void *entry_point, const process_level level,
                      const bool start_paused)
{
    auto process = new process_context;

    process->process_path = std::move(process_path);
    process->process_name = std::move(process_name);
    memset(process->sse_state, 0, sizeof(process->sse_state));
    process->is_paused = start_paused;
    process->is_being_debugged = false;
    process->debugger_pid = -1;
    process->level = level;

    // Allocate page map
    auto kvmem = allocate_virtual_memory_kernel(4096);
    auto process_page_map = allocate_physical_page();
    virtual_map(get_kernel_page_map(), reinterpret_cast<uint64_t>(process_page_map), kvmem, true, true, true, true, true, false, 0, true);
    process->page_map = reinterpret_cast<pml4 *>(kvmem);
    memset(process->page_map, 0, 4096);

    debug_print("Created process '%s' with page map at virt: %p, phys: %p\n",
                process->process_name.c_str(), process->page_map, process_page_map);

    // Set up thread
    process_thread th0{};
    th0.rip = reinterpret_cast<uint64_t>(entry_point);
    th0.rsp = 0x80000000 - 8; // Top of stack, aligned
    process->threads.push_back(th0);

    process->available_vmem.push_back({0x0, 0x80000000});

    // Allocate stack
    uint64_t stack_start_address = 0x80000000 - stack_size;
    auto vmem = allocate_virtual_memory_at(*process, stack_start_address, stack_size);
    if (!vmem.first)
    {
        debug_print("==[ERROR]== Failed to create process '%s'. Could not allocate stack.\n",
                    process->process_name.c_str());
        delete[] reinterpret_cast<uint8_t *>(process->page_map);
        return nullptr;
    }

    // Copy kernel upper half
    auto kernel_page_map = get_kernel_page_map();
    memcpy(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(process->page_map) + (sizeof(pml4) * 512) / 2),
           reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(kernel_page_map) + (sizeof(pml4) * 512) / 2),
           (sizeof(pml4) * 512) / 2);

    bool is_supervisor = (level == KERNEL);
    for (size_t page_index = 0; (stack_size / PAGE_SIZE) > page_index; page_index++)
    {
        const auto page_address = reinterpret_cast<uint64_t>(allocate_physical_page());
        if (!page_address)
        {
            debug_print("==[ERROR]== Failed to create process '%s'. Could not allocate stack page.\n",
                        process->process_name.c_str());
            delete[] reinterpret_cast<uint8_t *>(process->page_map);
            return nullptr;
        }
        virtual_map(process->page_map, page_address, stack_start_address + (page_index * PAGE_SIZE), true, true,
                    is_supervisor, true, true, false, 0, true);
    }

    // Allocate region for 0x10000 explicitly (assuming caller expects this)
    if (!allocate_memory_in_process(process, 0x10000, 1))
    {
        debug_print("==[WARNING]== Failed to allocate initial memory at 0x10000 for process '%s'.\n",
                    process->process_name.c_str());
    }

    debug_print("Process '%s' created with stack at 0x%lx, thread RIP: 0x%lx, RSP: 0x%lx\n",
                process->process_name.c_str(), stack_start_address, th0.rip, th0.rsp);
    return process;
}

std::pair<bool, uint64_t> allocate_virtual_memory_at(process_context &ctx, uint64_t requested_base, size_t size)
{
    for (auto &entry : ctx.available_vmem)
    {
        if (entry.base <= requested_base && (requested_base + size) <= (entry.base + entry.length))
        {
            uint64_t allocated_address = requested_base;
            size_t remaining_length = (entry.base + entry.length) - (requested_base + size);
            if (requested_base == entry.base)
            {
                entry.base += size;
                entry.length -= size;
            }
            else
            {
                if (remaining_length > 0)
                {
                    ctx.available_vmem.push_back({requested_base + size, remaining_length});
                }
                entry.length = requested_base - entry.base;
            }
            return {true, allocated_address};
        }
    }
    debug_print("==[ERROR]== No available virtual memory for base 0x%lx, size %lu\n", requested_base, size);
    return {false, 0};
}

std::pair<bool, uint64_t> allocate_virtual_memory(process_context &ctx, size_t size)
{
    for (auto it = ctx.available_vmem.begin(); it != ctx.available_vmem.end(); ++it)
    {
        if (it->length >= size)
        {
            uint64_t allocated_address = it->base;
            it->base += size;
            it->length -= size;
            if (it->length == 0)
            {
                ctx.available_vmem.erase(it);
            }
            return {true, allocated_address};
        }
    }
    return {false, 0};
}

bool allocate_memory_in_process(handle process, uint64_t at_address, uint64_t page_count)
{
    if ((at_address + (page_count * PAGE_SIZE)) > 0x80000000)
    {
        debug_print("==[ERROR]== Cannot allocate memory at 0x%lx: overlaps kernel space.\n", at_address);
        return false;
    }

    bool is_supervisor = (static_cast<process_context *>(process)->level == KERNEL);
    for (size_t page_index = 0; page_index < page_count; page_index++)
    {
        const auto page_address = reinterpret_cast<uint64_t>(allocate_physical_page());
        if (!page_address)
        {
            debug_print("==[ERROR]== Failed to allocate physical page for process memory at 0x%lx.\n",
                        at_address + (page_index * PAGE_SIZE));
            return false;
        }
        virtual_map(static_cast<process_context *>(process)->page_map, page_address,
                    at_address + (page_index * PAGE_SIZE), true, true, is_supervisor, true, true, false, 0, true);
    }
    debug_print("Allocated %lu pages at 0x%lx for process\n", page_count, at_address);
    return true;
}

bool process_write(handle process, uint64_t at_address, void *buffer, size_t buffer_size)
{
    DEBUG_PAUSE
    ENTER_CRITICAL_SECTION
    auto hProcess = static_cast<process_context *>(process);

    if (at_address >= 0x80000000 || (at_address + buffer_size) > 0x80000000)
    {
        debug_print("==[ERROR]== Access denied: Attempted write to kernel memory at 0x%lx (size: %lu).\n",
                    at_address, buffer_size);
        LEAVE_CRITICAL_SECTION
        return false;
    }

    debug_print("Writing to process '%s' at user-space address 0x%lx, size %lu\n",
                hProcess->process_name.c_str(), at_address, buffer_size);

    auto original_page_map = static_cast<pml4 *>(get_current_page_map_address());
    debug_print("Switching to process page map at %p\n", hProcess->page_map);
    auto isPMSet = set_page_map(hProcess->page_map);
    if (!isPMSet)
    {
        debug_print("==[ERROR]== Failed to set page map to %p for process '%s'.\n",
                    hProcess->page_map, hProcess->process_name.c_str());
        set_page_map(original_page_map);
        LEAVE_CRITICAL_SECTION
        return false;
    }

    debug_print("Invalidating pages from 0x%lx to 0x%lx\n", at_address, at_address + buffer_size);
    auto isInvalidated = invalidate_pages_in_range(at_address, at_address + buffer_size);
    if (!isInvalidated)
    {
        debug_print("==[ERROR]== Failed to invalidate pages for range 0x%lx–0x%lx.\n",
                    at_address, at_address + buffer_size);
        set_page_map(original_page_map);
        LEAVE_CRITICAL_SECTION
        return false;
    }

    debug_print("Verifying memory flags for range 0x%lx–0x%lx\n", at_address, at_address + buffer_size);
    bool isRight = ensure_virtual_memory_flags_in_range(hProcess->page_map, at_address, at_address + buffer_size,
                                                        [](const pt &page_ent) -> bool { return page_ent.rw && page_ent.p; });
    if (!isRight)
    {
        debug_print("==[ERROR]== Pages in range 0x%lx–0x%lx are not present or writable.\n",
                    at_address, at_address + buffer_size);
        set_page_map(original_page_map);
        LEAVE_CRITICAL_SECTION
        return false;
    }

    memcpy(reinterpret_cast<void *>(at_address), buffer, buffer_size);
    debug_print("Successfully wrote %lu bytes to 0x%lx\n", buffer_size, at_address);

    set_page_map(original_page_map);
    LEAVE_CRITICAL_SECTION
    return true;
}

bool process_read(handle process, uint64_t from_address, void *buffer, size_t buffer_size)
{
    DEBUG_PAUSE
    ENTER_CRITICAL_SECTION
    auto hProcess = static_cast<process_context *>(process);

    if (from_address >= 0x80000000 || (from_address + buffer_size) > 0x80000000)
    {
        debug_print("==[ERROR]== Access denied: Attempted read from kernel memory at 0x%lx (size: %lu).\n",
                    from_address, buffer_size);
        LEAVE_CRITICAL_SECTION
        return false;
    }

    debug_print("Reading from process '%s' at user-space address 0x%lx, size %lu\n",
                hProcess->process_name.c_str(), from_address, buffer_size);

    auto original_page_map = static_cast<pml4 *>(get_current_page_map_address());
    debug_print("Switching to process page map at %p\n", hProcess->page_map);
    auto isPMSet = set_page_map(hProcess->page_map);
    if (!isPMSet)
    {
        debug_print("==[ERROR]== Failed to set page map to %p for process '%s'.\n",
                    hProcess->page_map, hProcess->process_name.c_str());
        set_page_map(original_page_map);
        LEAVE_CRITICAL_SECTION
        return false;
    }

    debug_print("Invalidating pages from 0x%lx to 0x%lx\n", from_address, from_address + buffer_size);
    auto isInvalidated = invalidate_pages_in_range(from_address, from_address + buffer_size);
    if (!isInvalidated)
    {
        debug_print("==[ERROR]== Failed to invalidate pages for range 0x%lx–0x%lx.\n",
                    from_address, from_address + buffer_size);
        set_page_map(original_page_map);
        LEAVE_CRITICAL_SECTION
        return false;
    }

    debug_print("Verifying memory flags for range 0x%lx–0x%lx\n", from_address, from_address + buffer_size);
    bool isRight = ensure_virtual_memory_flags_in_range(hProcess->page_map, from_address, from_address + buffer_size,
                                                        [](const pt &page_ent) -> bool { return page_ent.p; });
    if (!isRight)
    {
        debug_print("==[ERROR]== Pages in range 0x%lx–0x%lx are not present.\n",
                    from_address, from_address + buffer_size);
        set_page_map(original_page_map);
        LEAVE_CRITICAL_SECTION
        return false;
    }

    memcpy(buffer, reinterpret_cast<void *>(from_address), buffer_size);
    debug_print("Successfully read %lu bytes from 0x%lx\n", buffer_size, from_address);

    set_page_map(original_page_map);
    LEAVE_CRITICAL_SECTION
    return true;
}