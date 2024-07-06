//
// Created by Piotr on 02.06.2024.
//

#ifndef KITTYKERNEL_CXA_GUARD_HPP
#define KITTYKERNEL_CXA_GUARD_HPP

#include <atomic>

struct cxa_guard
{
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    bool initialized = false;
};

#endif //KITTYKERNEL_CXA_GUARD_HPP
