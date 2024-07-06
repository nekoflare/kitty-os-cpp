//
// Created by Piotr on 05.06.2024.
//

#include "clock.hpp"

constexpr size_t pit_frequency = 200;
uint64_t clk = 0;

void clk_handler([[maybe_unused]] Registers_x86_64* regs)
{
    clk++;
}

double clk_get_time()
{
    double _Clk = clk;
    double _Freq = pit_frequency;
    double _T = _Clk * (1 / _Freq);

    return _T;
}

// t - seconds
void clk_sleep(double t) {
    // Get the start time
    double end_time = clk_get_time() + t;

    while (end_time > clk_get_time())
    {
        asm volatile ("nop");
    }
}

void clk_init()
{
    pit_init(pit_frequency);
    uirq_register_irq(0, &clk_handler);
    uirq_unmask_irq(0);
}