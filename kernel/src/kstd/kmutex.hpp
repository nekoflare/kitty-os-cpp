#ifndef KMUTEX_HPP
#define KMUTEX_HPP

#include <atomic>
#include <functional>

namespace kstd
{
    class mutex {
    public:
        mutex() : flag(ATOMIC_FLAG_INIT) {}

        // Lock the mutex (blocking)
        void lock();

        // Unlock the mutex
        void unlock();

    private:
        std::atomic_flag flag;
    };

    template<typename Func, typename... Args>
    auto with_lock(mutex &mutex, Func&& func, Args&&... args) -> decltype(func(std::forward<Args>(args)...)) {
        // Lock the mutex
        mutex.lock();

        // Call the function
        auto result = func(std::forward<Args>(args)...);

        // Unlock the mutex
        mutex.unlock();

        return result;
    }
}

#endif // KMUTEX_HPP
