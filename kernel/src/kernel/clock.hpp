//
// Created by Piotr on 05.06.2024.
//

#ifndef KITTY_OS_CPP_CLOCK_HPP
#define KITTY_OS_CPP_CLOCK_HPP

#include <devices/pit/pit.hpp>
#include <kernel/irqs/uniirq.hpp>

typedef struct _ktime_t
{
    size_t second, minute, hour;
    size_t day, month, year;
} ktime_t;

void clk_init();
double clk_get_time();
void clk_sleep(double t);

#endif //KITTY_OS_CPP_CLOCK_HPP
