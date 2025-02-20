//
// Created by Neko on 25.01.2025.
//

#ifndef CLOCK_H
#define CLOCK_H

#include <cstdint>
#include <functional>
#include <kernel.h>

void initialize_clock();

/**
 * Registers the callback and calls it every "ms" seconds.
 * @param callback - A function to be called every "ms" milliseconds.
 * @param ms - Milliseconds
 * @return - A handle to delete the callback in the future.
 */
handle listen_on_clock(const std::function<void()> &callback, uint64_t ms);

void delete_clock(handle clock);

#endif // CLOCK_H
