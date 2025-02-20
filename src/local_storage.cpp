//
// Created by Neko on 25.01.2025.
//

#include "local_storage.h"

#include <cstdint>
#include <hal/cpu/msr.h>
#include <kernel.h>
#include <lib/klibc/spinlock.h>
#include <mem/physical.h>
#include <smp/smp.h>
#include <vector>

struct storage_entry
{
    uint64_t uid;
    uint64_t processor_id;
    void *data_ptr;
};

static std::vector<storage_entry> global_storage;
static std::vector<storage_entry> storage;

handle create_storage_entry_spinlock;
handle get_storage_value_spinlock;

void initialize_local_storage()
{
    create_storage_entry_spinlock = create_spinlock();
    get_storage_value_spinlock = create_spinlock();

    // write to the gs.
    const auto tls = new thread_local_storage;
    tls->stack_ptr = reinterpret_cast<uint64_t>(new char[PAGE_SIZE * 32]);
    wrmsr(0xC0000101, reinterpret_cast<uint64_t>(tls));
    wrmsr(0xC0000102, reinterpret_cast<uint64_t>(tls));
}

void create_storage_entry(bool global, uint64_t stor_id, void *data)
{
    lock_spinlock(create_storage_entry_spinlock);
    if (global)
        global_storage.push_back({stor_id, get_current_processor_id(), data});
    else
        storage.push_back({stor_id, get_current_processor_id(), data});
    unlock_spinlock(create_storage_entry_spinlock);
}

void *get_storage_value(const bool global, const uint64_t stor_id)
{
    if (global)
    {
        lock_spinlock(get_storage_value_spinlock);
        for (const auto &e : global_storage)
        {
            if (e.uid == stor_id)
            {
                unlock_spinlock(get_storage_value_spinlock);
                return e.data_ptr;
            } // We don't check for the processor id because it's global data.
        }
    }
    else
    {
        for (const auto &[uid, processor_id, data_ptr] : storage)
        {
            if (uid == stor_id && processor_id == get_current_processor_id())
            {
                return data_ptr;
            }
        } // doesn't require spinlock.
    }

    return nullptr;
}