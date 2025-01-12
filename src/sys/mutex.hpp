#pragma once

#include <atomic>

class Mutex
{
  private:
    std::atomic<bool> locked;

  public:
    Mutex() : locked(false)
    {
    }

    void lock()
    {
        while (locked.exchange(true, std::memory_order_acquire))
        {
            // busy wait
            asm volatile("nop");
        }
    }

    void unlock()
    {
        locked.store(false, std::memory_order_release);
    }
};