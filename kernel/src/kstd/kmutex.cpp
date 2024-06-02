#include "kmutex.hpp"

namespace kstd
{
    void mutex::lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // Busy-wait until the lock is acquired
        }
    }

    void mutex::unlock() {
        flag.clear(std::memory_order_release);
    }
}
