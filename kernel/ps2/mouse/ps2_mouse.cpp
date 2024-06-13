//
// Created by Piotr on 03.06.2024.
//

#include <kernel/irqs/uniirq.hpp>
#include "ps2_mouse.hpp"

void ps2_mouse_handler(Registers_x86_64* regs)
{
    kstd::printf("ps2 packet received\n");
}

void ps2_mouse_wait(char type)
{
    uint32_t timeout = 100000;
    if (type == 0)
    {
        while (timeout--)
        {
            if ((inb(PS2_MOUSE_COMMAND_PORT) & 1) == 1)
            {
                return;
            }
        }
        return;
    }
    else
    {
        while (timeout--)
        {
            if ((inb(PS2_MOUSE_COMMAND_PORT) & 2) == 0)
            {
                return;
            }
        }
        return;
    }
}

void ps2_mouse_write(char v)
{
    ps2_mouse_wait(1);
    outb(MOUSE_STATUS, MOUSE_WRITE);
    ps2_mouse_wait(1);
    outb(MOUSE_PORT, v);
}

char ps2_mouse_read()
{
    ps2_mouse_wait(0);
    return inb(MOUSE_PORT);
}

void ps2_mouse_init()
{
    uint8_t status;
    ps2_mouse_wait(1);
    outb(MOUSE_STATUS, 0xA8);
    ps2_mouse_wait(1);
    outb(MOUSE_STATUS, 0x20);
    ps2_mouse_wait(0);
    status = inb(0x60) | 2;
    ps2_mouse_wait(1);
    outb(MOUSE_STATUS, 0x60);
    ps2_mouse_wait(1);
    outb(MOUSE_PORT, status);
    ps2_mouse_write(200);
    ps2_mouse_read();
    ps2_mouse_write(0xF4);
    ps2_mouse_read();


    uirq_register_irq(12, &ps2_mouse_handler);
    uirq_unmask_irq(12);
}