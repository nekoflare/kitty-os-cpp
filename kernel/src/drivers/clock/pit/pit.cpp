//
// Created by Piotr on 04.07.2024.
//

#include <drivers/clock/pit/pit.hpp>

void pit_init(const size_t desired_frequency)
{
    const uint16_t divisor = static_cast<uint16_t>(PIT_FREQUENCY / desired_frequency);

    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL_0, static_cast<uint8_t>(divisor & 0xff));
    outb(PIT_CHANNEL_0, static_cast<uint8_t>((divisor >> 8) & 0xff));
}