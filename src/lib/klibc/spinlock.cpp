//
// Created by Neko on 25.01.2025.
//

#include "spinlock.h"
#include <atomic>

struct spinlock
{
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

handle create_spinlock()
{
    return new spinlock();
}

void lock_spinlock(handle lock)
{
    auto sl = static_cast<spinlock *>(lock);
    while (sl->flag.test_and_set(std::memory_order_acquire))
    {
        asm volatile("cli; hlt");
    }
}

void unlock_spinlock(const handle lock)
{
    const auto sl = static_cast<spinlock *>(lock);
    sl->flag.clear(std::memory_order_release);
}

void destroy_spinlock(const handle lock)
{
    delete static_cast<spinlock *>(lock);
}
