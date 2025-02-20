//
// Created by Neko on 21.01.2025.
//

#ifndef KERNEL_H
#define KERNEL_H

#include <cstdint>

using status = int64_t;
using handle = void *;

#define DEBUG_BOCHS
#ifdef DEBUG_BOCHS
    #define DEBUG_PAUSE asm volatile ("xchg %bx, %bx");
#endif
#ifndef DEBUG_BOCHS
    #define DEBUG_PAUSE
#endif

#define ENTER_CRITICAL_SECTION asm volatile("cli"); // disable interrupts
#define LEAVE_CRITICAL_SECTION asm volatile("sti"); // enable interrupts

constexpr status STATUS_SUCCESS = 0;
constexpr status STATUS_FAILURE = -1;
constexpr status STATUS_INVALID_ARGUMENTS = -2;
constexpr status STATUS_DOESNT_EXIST = -3;
constexpr status STATUS_UNKNOWN_ERROR = -4;
constexpr status STATUS_OUT_OF_RESOURCES = -5;

#endif // KERNEL_H
