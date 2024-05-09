//
// Created by Piotr on 05.05.2024.
//

#include "sb16.hpp"

bool is_sb16_available()
{
    outb(DSP_RESET, 1);
    iowait(1);
    outb(DSP_RESET, 0);
    iowait(1);

    if (inb(DSP_READ) != 0xAA)
    {
        return false;
    }

    return true;
}

void dsp_write(uint8_t value)
{
    size_t status;

    status = static_cast<size_t>(inb(DSP_WRITE));

    outb(DSP_WRITE, value);
}

uint8_t dsp_read()
{
    return inb(DSP_READ);
}
