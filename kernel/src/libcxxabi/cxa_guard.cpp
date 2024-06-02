//
// Created by Piotr on 02.06.2024.
//

#include "cxa_guard.hpp"

extern "C"
{
    int __cxa_guard_acquire(struct cxa_guard* guard_object)
    {
        while (guard_object->flag.test_and_set(std::memory_order_acquire))
        {
            // Busy-wait until the lock is acquired
        }
        if (guard_object->initialized)
        {
            guard_object->flag.clear(std::memory_order_release);
            return 0; // Already initialized
        }
        return 1; // Need initialization
    }

    // Function to release the guard
   void __cxa_guard_release(struct cxa_guard* guard_object)
   {
        guard_object->initialized = true;
        guard_object->flag.clear(std::memory_order_release);
   }
}